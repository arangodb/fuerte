////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Christoph Uhde
/// @author Ewout Prangsma
////////////////////////////////////////////////////////////////////////////////

#include <condition_variable>

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <fuerte/FuerteLogger.h>
#include <fuerte/helper.h>
#include <fuerte/loop.h>
#include <fuerte/message.h>
#include <fuerte/types.h>

#include "VstConnection.h"
#include "vst.h"

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {

using namespace arangodb::fuerte::detail;

namespace ba = ::boost::asio;
namespace bs = ::boost::asio::ssl;
using bt = ::boost::asio::ip::tcp;
using be = ::boost::asio::ip::tcp::endpoint;
using BoostEC = ::boost::system::error_code;
using RequestItemSP = std::shared_ptr<RequestItem>;
using Lock = std::lock_guard<std::mutex>;
typedef std::unique_ptr<Request> RequestUP;
typedef std::unique_ptr<Response> ResponseUP;

// sendRequest prepares a RequestItem for the given parameters
// and adds it to the send queue.
MessageID VstConnection::sendRequest(std::unique_ptr<Request> request, RequestCallback cb) {

  //check if id is already used and fail
  request->messageID = ++_messageID;
  auto item = std::make_shared<RequestItem>();

  item->_messageID = request->messageID;
  item->_callback = cb;
  item->_request = std::move(request);
  item->prepareForNetwork(_vstVersion);

  // Add item to send queue
  _sendQueue.add(item);

  // this allows sendRequest to return immediately and
  // not to block until all writing is done
  if (_connected) {
    FUERTE_LOG_VSTTRACE << "start sending & reading" << std::endl;
    startWriting();
  } else {
    FUERTE_LOG_VSTTRACE << "sendRequest (async): not connected" << std::endl;
  }

  FUERTE_LOG_VSTTRACE << "sendRequest (async) done" << std::endl;
  return item->_messageID;
}

std::size_t VstConnection::requestsLeft() {
  // this function does not return the exact size (both mutexes would be
  // required to be locked at the same time) but as it is used to decide
  // if another run is called or not this should not be critical.
  return _sendQueue.size() + _messageStore.size();
};

VstConnection::VstConnection(EventLoopService& eventLoopService, ConnectionConfiguration const& configuration)
    : Connection(eventLoopService, configuration)
    , _vstVersion(configuration._vstVersion)
    , _messageID(0)
    , _ioService(eventLoopService.io_service())
    , _resolver(new bt::resolver(*eventLoopService.io_service()))
    , _socket(nullptr)
    , _context(bs::context::method::sslv23)
    , _sslSocket(nullptr)
    , _connected(false)
    , _async_calls(0)
{
    assert(!_readLoop._current);
    assert(!_writeLoop._current);
}

// Deconstruct.
VstConnection::~VstConnection() {
  _resolver->cancel();
  shutdownConnection();
}

// Activate this connection.
void VstConnection::start() {
  startResolveHost();
}

// resolve the host into a series of endpoints 
void VstConnection::startResolveHost() {
  // Resolve the host asynchronous.
  auto self = shared_from_this();
  _resolver->async_resolve({_configuration._host, _configuration._port},
    [this, self](const boost::system::error_code& error, bt::resolver::iterator iterator) {
      if (error) {
        FUERTE_LOG_ERROR << "resolve failed: error=" << error << std::endl;
        onFailure(errorToInt(ErrorCondition::CouldNotConnect), "resolved failed: error" + error.message());
      } else {
        FUERTE_LOG_CALLBACKS << "resolve succeeded" << std::endl;
        _endpoints = iterator;
        if (_endpoints == bt::resolver::iterator()){
          FUERTE_LOG_ERROR << "unable to resolve endpoints" << std::endl;
          onFailure(errorToInt(ErrorCondition::CouldNotConnect), "unable to resolve endpoints");
        } else {
          initSocket();
        }
      }
    });
}

// CONNECT RECONNECT //////////////////////////////////////////////////////////

void VstConnection::initSocket() {
  std::lock_guard<std::mutex> lock(_socket_mutex);

  // socket must be empty before. Check that 
  assert(!_socket);
  assert(!_sslSocket);

  FUERTE_LOG_CALLBACKS << "begin init" << std::endl;
  _socket.reset(new bt::socket(*_ioService));
  _sslSocket.reset(new bs::stream<bt::socket&>(*_socket, _context));
  
  auto endpoints = _endpoints; // copy as connect modifies iterator
  startConnect(endpoints);
}

// close the TCP & SSL socket.
void VstConnection::shutdownSocket() {
  std::lock_guard<std::mutex> lock(_socket_mutex);

  FUERTE_LOG_CALLBACKS << "begin shutdown socket" << std::endl;

  BoostEC error;
  if (_sslSocket) {
    _sslSocket->shutdown(error);
  }
  if (_socket) {
    _socket->cancel();
    _socket->shutdown(bt::socket::shutdown_both, error);
    _socket->close(error);
  }
  _sslSocket = nullptr;
  _socket = nullptr;
}

// shutdown the connection and cancel all pending messages.
void VstConnection::shutdownConnection() {
  FUERTE_LOG_CALLBACKS << "shutdownConnection" << std::endl;

  // Stop the read & write loop 
  stopWriting();
  stopReading();

  // Close socket
  shutdownSocket();

  // Cancel all items and remove them from the message store.
  _messageStore.cancelAll();
}

void VstConnection::restartConnection(){
  assert(false);
  FUERTE_LOG_CALLBACKS << "restartConnection" << std::endl;
  shutdownConnection();
  startResolveHost();
}

// ------------------------------------
// Creating a connection
// ------------------------------------

// try to open the socket connection to the first endpoint.
void VstConnection::startConnect(bt::resolver::iterator endpointItr){
  if (endpointItr != boost::asio::ip::tcp::resolver::iterator()) {
    FUERTE_LOG_CALLBACKS << "trying to connect to: " << endpointItr->endpoint() << "..." << std::endl;

    // Set a deadline for the connect operation.
    //_deadline.expires_from_now(boost::posix_time::seconds(60));
    // TODO wait for connect timeout

    // Start the asynchronous connect operation.
    auto self = shared_from_this();
    ba::async_connect(*_socket, endpointItr, 
      boost::bind(&VstConnection::asyncConnectCallback, std::dynamic_pointer_cast<VstConnection>(self), _1, endpointItr));
  }
}

// callback handler for async_callback (called in startConnect).
void VstConnection::asyncConnectCallback(BoostEC const& error, bt::resolver::iterator endpointItr) {
  if (error) {
    // Connection failed
    FUERTE_LOG_ERROR << error.message() << std::endl;
    shutdownConnection();
    if(endpointItr == bt::resolver::iterator()) {
      FUERTE_LOG_CALLBACKS << "no further endpoint" << std::endl;
    }
    onFailure(errorToInt(ErrorCondition::CouldNotConnect), "unable to connect -- " + error.message());
  } else {
    // Connection established
    FUERTE_LOG_CALLBACKS << "TCP socket connected" << std::endl;
    if(_configuration._ssl) {
      startSSLHandshake();
    } else {
      finishInitialization();
    }
  }
}

// socket connection is up (with optional SSL), now initiate the VST protocol.
void VstConnection::finishInitialization(){
  FUERTE_LOG_CALLBACKS << "finishInitialization" << std::endl;

  const char* vstHeader;
  switch (_vstVersion) {
    case VST1_0:
      vstHeader = vstHeader1_0;
      break;
    case VST1_1:
      vstHeader = vstHeader1_1;
      break;
    default:
      throw std::logic_error("Unknown VST version");
  }

  auto self = shared_from_this();
  ba::async_write(*_socket
                 ,ba::buffer(vstHeader, strlen(vstHeader))
                 ,[this,self](BoostEC const& error, std::size_t transferred){
                    if (error) {
                      FUERTE_LOG_ERROR << error.message() << std::endl;
                      _connected = false;
                      shutdownConnection();
                      onFailure(errorToInt(ErrorCondition::CouldNotConnect), "unable to initialize connection: error=" + error.message());
                    } else {
                      FUERTE_LOG_CALLBACKS << "VST connection established; starting send/read loop" << std::endl;
                      _connected = true;
                      startWriting();
                      startReading();
                    }
                  }
                 );
}

// start intiating an SSL connection (on top of an established TCP socket)
void VstConnection::startSSLHandshake() {
  if(!_configuration._ssl){
    finishInitialization();
  }
  FUERTE_LOG_CALLBACKS << "starting ssl handshake " << std::endl;
  auto self = shared_from_this();
  _sslSocket->async_handshake(
      bs::stream_base::client, [this, self](BoostEC const& error) {
        if (error) {
          shutdownSocket();
          FUERTE_LOG_ERROR << error.message() << std::endl;
          onFailure(errorToInt(ErrorCondition::CouldNotConnect),
            "unable to perform ssl handshake: error=" + error.message());
        } else {
          FUERTE_LOG_CALLBACKS << "ssl handshake done" << std::endl;
          finishInitialization();
        }
      });
}

// ------------------------------------
// Reading data
// ------------------------------------

// activate the receiver loop (if needed)
void VstConnection::startReading() {
  ReadLoop *newLoop;
  {
    std::lock_guard<std::mutex> lock(_readLoop._mutex);
    if (_readLoop._current) {
      // There is already a read loop, do nothing 
      return;
    }
    // There is no current read loop, create one 
    _readLoop._current = std::make_shared<ReadLoop>(std::dynamic_pointer_cast<VstConnection>(shared_from_this()), _socket);
    newLoop = _readLoop._current.get();
  }
  // Start the new loop
  newLoop->start();
}

// Stop the current read loop
void VstConnection::stopReading() {
  {
    std::lock_guard<std::mutex> lock(_readLoop._mutex);
    _readLoop._current.reset();
  }
}

// called by a ReadLoop to decide if it must stop.
// returns true when the given loop should stop.
bool VstConnection::shouldStopReading(const ReadLoop* readLoop) {
  // Claim exclusive access 
  std::unique_lock<std::mutex> readLoopLock(_readLoop._mutex, std::defer_lock);
  std::unique_lock<std::mutex> queueLock(_sendQueue.mutex(), std::defer_lock);
  std::unique_lock<std::mutex> mapLock(_messageStore.mutex(), std::defer_lock);
  std::lock(readLoopLock, queueLock, mapLock);

  // Is the read loop still the current read loop?
  if (_readLoop._current.get() != readLoop) {
    FUERTE_LOG_VSTTRACE << "shouldStopReading: no longer current loop: loop=" << readLoop << std::endl;
    return true;
  }

  // Is there any work left for the read loop?
  if (_messageStore.empty(true) && _sendQueue.empty(true)) {
    // No more work 
    _readLoop._current.reset();
    FUERTE_LOG_VSTTRACE << "shouldStopReading: no more pending messages/requests, stopping read loop: loop=" << readLoop << std::endl;
    return true;
  }

  // Continue read loop
  return false;
}

// start the read loop 
void VstConnection::ReadLoop::start() {
  auto wasStarted = _started.exchange(true);
  if (!wasStarted) {
    readNextBytes();
  }
}

// readNextBytes reads the next bytes from the server.
void VstConnection::ReadLoop::readNextBytes() {
  FUERTE_LOG_VSTTRACE << "readNextBytes: this=" << this << std::endl;
  FUERTE_LOG_CALLBACKS << "-";

  // Ask the connection if we should terminate.
  if (_connection->shouldStopReading(this)) {
    FUERTE_LOG_VSTTRACE << "readNextBytes: stopping read loop" << std::endl;
    return;    
  }

  // Start reading data from the network.
  _deadline.expires_from_now(boost::posix_time::seconds(30));
  // TODO async_wait for timeout 

  FUERTE_LOG_CALLBACKS << "r";
#if ENABLE_FUERTE_LOG_CALLBACKS > 0
  std::cout << "_messageMap = " << _connection->_messageStore.keys() << std::endl;
#endif
  auto self = shared_from_this();
  _connection->_async_calls++;
  ba::async_read(*_socket, _receiveBuffer, ba::transfer_at_least(1),
    boost::bind(&ReadLoop::asyncReadCallback, self, _1, _2));

  FUERTE_LOG_VSTTRACE << "readNextBytes: done" << std::endl;
}

// asyncReadCallback is called when readNextBytes is resulting in some data.
void VstConnection::ReadLoop::asyncReadCallback(const boost::system::error_code& error, std::size_t transferred) {
  auto pendingAsyncCalls = --_connection->_async_calls;
  if (error) {
    FUERTE_LOG_CALLBACKS << "asyncReadCallback: Error while reading form socket";
    FUERTE_LOG_ERROR << error.message() << std::endl;

    // Restart connection, this will trigger a release of the readloop.
    _connection->restartConnection(); 
  } else {
    FUERTE_LOG_CALLBACKS << "asyncReadCallback: received " << transferred << " bytes async-calls=" << pendingAsyncCalls << std::endl;

    // Inspect the data we've received so far.
    auto receivedBuf = _receiveBuffer.data(); // no copy
    auto cursor = boost::asio::buffer_cast<const uint8_t*>(receivedBuf);
    auto available = boost::asio::buffer_size(receivedBuf);
    while (vst::isChunkComplete(cursor, available)) {
      // Read chunk 
      ChunkHeader chunk;
      switch (_connection->_vstVersion) {
        case VST1_0:
          chunk = vst::readChunkHeaderVST1_0(cursor);
          break;
        case VST1_1:
          chunk = vst::readChunkHeaderVST1_1(cursor);
          break;
        default:
          throw std::logic_error("Unknown VST version");
      }

      // Process chunk 
      _connection->processChunk(chunk);

      // Remove consumed data from receive buffer.
      _receiveBuffer.consume(chunk.chunkLength());
      cursor += chunk.chunkLength();
      available -= chunk.chunkLength();
    }

    // Continue reading data
    readNextBytes();
  }
}

// Process the given incoming chunk.
void VstConnection::processChunk(ChunkHeader &chunk) {
  auto msgID = chunk.messageID();
  FUERTE_LOG_VSTTRACE << "processChunk: messageID=" << msgID << std::endl;

  // Find requestItem for this chunk.  
  auto item = _messageStore.findByID(chunk._messageID);
  if (!item) {
    FUERTE_LOG_ERROR << "got chunk with unknown message ID: " << msgID << std::endl;
    return;
  }

  // We've found the matching RequestItem.
  item->addChunk(chunk);

  // Try to assembly chunks in RequestItem to complete response.
  auto completeBuffer = item->assemble();
  if (completeBuffer) {
    FUERTE_LOG_VSTTRACE << "processChunk: complete response received" << std::endl;
    // Message is complete 
    // Remove message from store 
    _messageStore.removeByID(item->_messageID);

    // Create response
    auto response = createResponse(*item, completeBuffer);

    // Notify listeners
    FUERTE_LOG_VSTTRACE << "processChunk: notifying RequestItem onSuccess callback" << std::endl;
    item->_callback.invoke(0, std::move(item->_request), std::move(response));
  }
}

// Create a response object for given RequestItem & received response buffer.
std::unique_ptr<Response> VstConnection::createResponse(RequestItem& item, std::unique_ptr<VBuffer>& responseBuffer) {
  FUERTE_LOG_VSTTRACE << "creating response for item with messageid: " << item._messageID << std::endl;
  auto itemCursor = responseBuffer->data();
  auto itemLength = responseBuffer->byteSize();
  std::size_t messageHeaderLength;
  int vstVersionID = 1;
  MessageHeader messageHeader = validateAndExtractMessageHeader(vstVersionID, itemCursor, itemLength, messageHeaderLength);

  auto response = std::unique_ptr<Response>(new Response(std::move(messageHeader)));
  response->messageID = item._messageID;
  response->setPayload(std::move(*responseBuffer), messageHeaderLength);

  return response;
}

// ------------------------------------
// Writing data
// ------------------------------------

// activate the sender loop (if needed)
void VstConnection::startWriting() {
  WriteLoop *newLoop;
  {
    std::lock_guard<std::mutex> lock(_writeLoop._mutex);
    if (_writeLoop._current) {
      // There is already a write loop, do nothing 
      return;
    }
    // There is no current write loop, create one 
    _writeLoop._current = std::make_shared<WriteLoop>(std::dynamic_pointer_cast<VstConnection>(shared_from_this()), _socket);
    newLoop = _writeLoop._current.get();
  }
  // Start the new loop
  newLoop->start();
}

// Stop the current write loop
void VstConnection::stopWriting() {
  {
    std::lock_guard<std::mutex> lock(_writeLoop._mutex);
    _writeLoop._current.reset();
  }
}

// called by a WriteLoop to request for the next request that will be written.
// If there is no more work, nullptr is returned and the given loop must stop.
std::shared_ptr<RequestItem> VstConnection::getNextRequestToSend(const WriteLoop* writeLoop) {
  // Claim exclusive access 
  std::lock_guard<std::mutex> lock(_writeLoop._mutex);

  // Is the rwriteead loop still the current write loop?
  if (_writeLoop._current.get() != writeLoop) {
    FUERTE_LOG_VSTTRACE << "shouldStopWriting: no longer current loop: loop=" << writeLoop << std::endl;
    return std::shared_ptr<RequestItem>(nullptr);
  }

  // Get next request from send queue.
  auto next = _sendQueue.front();
  if (!next) {
    // send queue is empty
    FUERTE_LOG_VSTTRACE << "sendNextRequest: sendQueue empty" << std::endl;
    _writeLoop._current.reset();
    return std::shared_ptr<RequestItem>(nullptr);
  }

  // Add item to message store 
  _messageStore.add(next);

  // Remove item from send queue 
  _sendQueue.removeFirst();

  // Return message
  return next;
}

// start the write loop 
void VstConnection::WriteLoop::start() {
  auto wasStarted = _started.exchange(true);
  if (!wasStarted) {
    sendNextRequest();
  }
}

// writes data from task queue to network using boost::asio::async_write
void VstConnection::WriteLoop::sendNextRequest() {
  FUERTE_LOG_VSTTRACE << "sendNextRequest" << std::endl;
  FUERTE_LOG_TRACE << "+" ;

  // Get next request to send.
  auto next = _connection->getNextRequestToSend(this);
  if (!next) {
    // No more work for me.
    return;
  }

  // Make sure we're listening for a response 
  _connection->startReading();

  FUERTE_LOG_VSTTRACE << "sendNextRequest: preparing to send next" << std::endl;

  // make sure we are connected and handshake has been done
  auto self = shared_from_this();
  assert(next);
  assert(next->_requestBuffers.size());
/*#ifdef FUERTE_CHECKED_MODE
  FUERTE_LOG_VSTTRACE << "Checking outgoing data for message: " << next->_messageID << std::endl;
  auto vstChunkHeader = vst::readChunkHeaderV1_0(data.data());
  validateAndCount(data.data() + vstChunkHeader._chunkHeaderLength
                  ,data.byteSize() - vstChunkHeader._chunkHeaderLength);
#endif*/
  _connection->_async_calls++;
  ba::async_write(*_socket, 
    next->_requestBuffers,
    [this, self, next](BoostEC const& error, std::size_t transferred) {
      asyncWriteCallback(error, transferred, next);
    });

  FUERTE_LOG_VSTTRACE << "sendNextRequest: done" << std::endl;
}

// callback of async_write function that is called in sendNextRequest.
void VstConnection::WriteLoop::asyncWriteCallback(BoostEC const& error, std::size_t transferred, RequestItemSP item) {
  auto pendingAsyncCalls = --_connection->_async_calls;
  if (error) {
    // Send failed
    FUERTE_LOG_CALLBACKS << "asyncWriteCallback: error " << error.message() << std::endl;
    FUERTE_LOG_ERROR << error.message() << std::endl;

    // Item has failed, remove from message store
    _connection->_messageStore.removeByID(item->_messageID);

    // let user know that this request caused the error
    item->_callback.invoke(errorToInt(ErrorCondition::VstWriteError), std::move(item->_request), nullptr);

    // Stop current connection and try to restart a new one.
    // This will reset the current write loop.
    _connection->restartConnection();
  } else {
    // Send succeeded
    FUERTE_LOG_CALLBACKS << "asyncWriteCallback: send succeeded, " << transferred << " bytes transferred async-calls=" << pendingAsyncCalls << std::endl;

    // request is written we no longer data for that
    item->resetSendData();

    // Continue with next request (if any)
    FUERTE_LOG_CALLBACKS << "asyncWriteCallback: send next request (if any)" << std::endl;
    sendNextRequest();
  }
}
}}}}
