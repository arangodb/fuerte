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

#include "VstConnection.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <condition_variable>
#include <fuerte/FuerteLogger.h>
#include <fuerte/helper.h>
#include <fuerte/loop.h>
#include <fuerte/message.h>
#include <fuerte/vst.h>

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
MessageID VstConnection::sendRequest(std::unique_ptr<Request> request
                                    ,OnErrorCallback onError
                                    ,OnSuccessCallback onSuccess) {

  //check if id is already used and fail
  request->messageID = ++_messageID;
  auto item = std::make_shared<RequestItem>();

  item->_messageID = request->messageID;
  item->_onError = onError;
  item->_onSuccess = onSuccess;
  item->_request = std::move(request);
  item->prepareForNetwork(_vstVersion);

  // Add item to send queue
  _sendQueue.add(item);

  // this allows sendRequest to return immediately and
  // not to block until all writing is done
  if (_connected) {
    FUERTE_LOG_VSTTRACE << "start sending & reading" << std::endl;
    startSending();
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

std::unique_ptr<Response> VstConnection::sendRequest(RequestUP request){
  FUERTE_LOG_VSTTRACE << "start sync request" << std::endl;
  // TODO - we expect the loop to be running even for sync requests
  // maybe we could call poll on the ioservice in this function if
  // it is not running.
  std::mutex mutex;
  std::condition_variable conditionVar;
  bool done = false;

  auto rv = std::unique_ptr<Response>(nullptr);
  auto onError  = [&](::arangodb::fuerte::v1::Error error, RequestUP request, ResponseUP response){
    FUERTE_LOG_VSTTRACE << "sendRequest (sync): onError" << std::endl;
    rv = std::move(response);
    done = true;
    conditionVar.notify_one();
  };

  auto onSuccess  = [&](RequestUP request, ResponseUP response){
    FUERTE_LOG_VSTTRACE << "sendRequest (sync): onSuccess" << std::endl;
    rv = std::move(response);
    done = true;
    conditionVar.notify_one();
  };

  {
    std::unique_lock<std::mutex> lock(mutex);
    sendRequest(std::move(request),onError,onSuccess);
    /*if(!_asioLoop->_running){
      FUERTE_LOG_VSTTRACE << "sendRequest (sync): before run" << std::endl;
      _asioLoop->run_ready();
      //arangodb::fuerte::run(); // run
      FUERTE_LOG_VSTTRACE << "sendRequest (sync): after run" << std::endl;
    }*/
    FUERTE_LOG_VSTTRACE << "sendRequest (sync): before wait" << std::endl;
    conditionVar.wait(lock, [&]{ return done; });
  }
  FUERTE_LOG_VSTTRACE << "sendRequest (sync): done" << std::endl;
  return std::move(rv);
}


VstConnection::VstConnection(EventLoopService& eventLoopService, ConnectionConfiguration const& configuration)
    : _vstVersion(VST1_0)
    , _messageID(0)
    , _ioService(eventLoopService.io_service())
    , _socket(nullptr)
    , _context(bs::context::method::sslv23)
    , _sslSocket(nullptr)
    , _connected(false)
    , _connectionState(0)
    , _reading(false)
    , _sending(false)
    , _configuration(configuration)
    , _deadline(*_ioService)
{
    bt::resolver resolver(*_ioService);

    //TODO async resolve - set other endpoints
    _endpoints = resolver.resolve({configuration._host,configuration._port});
    if (_endpoints == bt::resolver::iterator()){
      throw std::runtime_error("unable to resolve endpoints");
    }

    //initSocket(); -- make_shared_from_this not allowed in constructor -- called after creation
}

// CONNECT RECONNECT //////////////////////////////////////////////////////////

void VstConnection::initSocket() {
  FUERTE_LOG_CALLBACKS << "begin init" << std::endl;
  _socket.reset(new bt::socket(*_ioService));
  _sslSocket.reset(new bs::stream<bt::socket&>(*_socket, _context));
  auto endpoints = _endpoints; // copy as connect modifies iterator
  startConnect(endpoints);
}

// close the TCP & SSL socket.
void VstConnection::shutdownSocket() {
  std::unique_lock<std::mutex> queueLock(_sendQueue.mutex(), std::defer_lock);
  std::unique_lock<std::mutex> mapLock(_messageStore.mutex(), std::defer_lock);
  std::lock(queueLock, mapLock);

  FUERTE_LOG_CALLBACKS << "begin shutdown socket" << std::endl;

  _deadline.cancel();
  BoostEC error;
  _sslSocket->shutdown(error);
  _socket->shutdown(bt::socket::shutdown_both,error);
  _socket->close(error);
  _sslSocket = nullptr;
  _socket = nullptr;
}

// shutdown the connection and cancel all pending messages.
void VstConnection::shutdownConnection() {
  FUERTE_LOG_CALLBACKS << "shutdownConnection" << std::endl;
  // Change state, so the send & read loop will terminate.
  _connectionState++;

  // Close socket
  shutdownSocket();

  // Cancel all items and remove them from the message store.
  _messageStore.cancelAll();
}

void VstConnection::restartConnection(){
  FUERTE_LOG_CALLBACKS << "restartConnection" << std::endl;
  shutdownConnection();
  initSocket();
}

// ASIO CONNECT

// try to open the socket connection to the first endpoint.
void VstConnection::startConnect(bt::resolver::iterator endpointItr){
  if (endpointItr != boost::asio::ip::tcp::resolver::iterator()) {
    FUERTE_LOG_CALLBACKS << "trying to connect to: " << endpointItr->endpoint() << "..." << std::endl;

    // Set a deadline for the connect operation.
    _deadline.expires_from_now(boost::posix_time::seconds(60));

    // Start the asynchronous connect operation.
    auto self = shared_from_this();
    ba::async_connect(*_socket
                     ,endpointItr
                     ,[this,self](BoostEC const& error, bt::resolver::iterator endpointItr){
                        asyncConnectCallback(error,endpointItr);
                      }
                     );
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
    throw std::runtime_error("unable to connect -- " + error.message() );
  } else {
    // Connection established
    FUERTE_LOG_CALLBACKS << "connected" << std::endl;
    if(_configuration._ssl) {
      startSSLHandshake();
    } else {
      finishInitialization();
    }
  }
}

// socket connection is up (with optional SSL), now initiate the VST protocol.
void VstConnection::finishInitialization(){
  FUERTE_LOG_CALLBACKS << "finish initialization" << std::endl;

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
                    } else {
                      FUERTE_LOG_CALLBACKS << "finish initialization; VST header sent" << std::endl;
                      _connectionState++;
                      _connected = true;
                      startSending();
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
  _sslSocket->async_handshake(bs::stream_base::client
                             ,[this,self](BoostEC const& error){
                               if(error){
                                 shutdownSocket();
                                 FUERTE_LOG_ERROR << error.message() << std::endl;
                                 throw std::runtime_error("unable to perform ssl handshake");
                               }
                               FUERTE_LOG_CALLBACKS << "ssl handshake done" << std::endl ;
                               finishInitialization();
                              }
                            );

}

// READING WRITING / NORMAL OPERATIONS  ///////////////////////////////////////

// readNextBytes reads the next bytes from the server.
void VstConnection::readNextBytes(int initialConnectionState) {
  FUERTE_LOG_VSTTRACE << "readNextBytes" << std::endl;
  FUERTE_LOG_CALLBACKS << "-";

  // Terminate the read loop if our connection state has changed.
  if (_connectionState != initialConnectionState) {
    FUERTE_LOG_VSTTRACE << "readNextBytes: connectionState changed, stopping read loop" << std::endl;
    _reading = false;
    return;
  }

  // Terminate the read loop if there is no more work for it.
  {
    std::unique_lock<std::mutex> queueLock(_sendQueue.mutex(), std::defer_lock);
    std::unique_lock<std::mutex> mapLock(_messageStore.mutex(), std::defer_lock);
    std::lock(queueLock, mapLock);
    if (_messageStore.empty(true) && _sendQueue.empty(true)) {
      _reading = false;
      FUERTE_LOG_VSTTRACE << "readNextBytes: no more pending messages/requests, stopping read loop" << std::endl;
      return;
    }
  }

  // Start reading data from the network.
  _deadline.expires_from_now(boost::posix_time::seconds(30));
  FUERTE_LOG_CALLBACKS << "r";
#if ENABLE_FUERTE_LOG_CALLBACKS > 0
  std::cout << "_messageMap = " << _messageStore.keys() << std::endl;
#endif
  auto self = shared_from_this();
  ba::async_read(*_socket
                ,_receiveBuffer
                ,ba::transfer_at_least(4)
                ,[this, self, initialConnectionState](const boost::system::error_code& error, std::size_t transferred){
                   asyncReadCallback(error, transferred, initialConnectionState);
                 }
                );

  FUERTE_LOG_VSTTRACE << "readNextBytes: done" << std::endl;
}

// asyncReadCallback is called when readNextBytes is resulting in some data.
void VstConnection::asyncReadCallback(const boost::system::error_code& error, std::size_t transferred, int initialConnectionState) {
  FUERTE_LOG_CALLBACKS << "asyncReadCallback begin" ;
  if (error) {
    FUERTE_LOG_CALLBACKS << "Error while reading form socket";
    FUERTE_LOG_ERROR << error.message() << std::endl;
    restartConnection();
  }

  FUERTE_LOG_CALLBACKS << "R(" << transferred << ")" ;

  if (_connectionState != initialConnectionState) {
    FUERTE_LOG_VSTTRACE << "asyncReadCallback: connectionState changed, stopping read loop" << std::endl;
    _reading = false;
    return;
  }

  // Inspect the data we've received so far.
  auto receivedBuf = _receiveBuffer.data(); // no copy
  auto cursor = boost::asio::buffer_cast<const uint8_t*>(receivedBuf);
  auto available = boost::asio::buffer_size(receivedBuf);
  while (vst::isChunkComplete(cursor, available)) {
    // Read chunk 
    ChunkHeader chunk;
    switch (_vstVersion) {
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
    processChunk(chunk);

    // Remove consumed data from receive buffer.
    _receiveBuffer.consume(chunk.chunkLength());
    cursor += chunk.chunkLength();
    available -= chunk.chunkLength();
  }

  // Continue reading data
  readNextBytes(initialConnectionState);
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
  FUERTE_LOG_VSTTRACE << "processChunk: found RequestItem: addChunk" << std::endl;
  item->addChunk(chunk);

  // Try to assembly chunks in RequestItem to complete response.
  FUERTE_LOG_VSTTRACE << "processChunk: found RequestItem: assemble" << std::endl;
  auto completeBuffer = item->assemble();
  if (completeBuffer) {
    // Message is complete 
    // Remove message from store 
    _messageStore.removeByID(item->_messageID);

    // Create response
    FUERTE_LOG_VSTTRACE << "processChunk: found RequestItem: createResponse" << std::endl;
    auto response = createResponse(*item, completeBuffer);

    // Notify listeners
    FUERTE_LOG_VSTTRACE << "processChunk: found RequestItem: onSuccess" << std::endl;
    item->_onSuccess(std::move(item->_request), std::move(response));
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

// writes data from task queue to network using boost::asio::async_write
void VstConnection::sendNextRequest(int initialConnectionState) {
  FUERTE_LOG_CALLBACKS << "sendNextRequest" << std::endl;
  FUERTE_LOG_TRACE << "+" ;

  // Has connection state changed, then stop.
  if (_connectionState != initialConnectionState) {
    FUERTE_LOG_CALLBACKS << "sendNextRequest: connection state changed, stopping send loop" << std::endl;
    _sending = false;
    return;
  }

  // Get next request from send queue.
  auto next = _sendQueue.front();
  if (!next) {
    // send queue is empty
    FUERTE_LOG_CALLBACKS << "sendNextRequest: sendQueue empty" << std::endl;
    _sending = false;
    return;
  }

  // Add item to message store 
  _messageStore.add(next);

  // Remove item from send queue 
  _sendQueue.removeFirst();

  // Make sure we're listening for a response 
  startReading();

  FUERTE_LOG_CALLBACKS << "sendNextRequest: preparing to send next" << std::endl;

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
  ba::async_write(*_socket, 
    next->_requestBuffers,
    [this, self, next, initialConnectionState](BoostEC const& error, std::size_t transferred) {
      asyncWriteCallback(error, transferred, next, initialConnectionState);
    });

  FUERTE_LOG_CALLBACKS << "sendNextRequest: done" << std::endl;
}

// callback of async_write function that is called in sendNextRequest.
void VstConnection::asyncWriteCallback(BoostEC const& error, std::size_t transferred, RequestItemSP item, int initialConnectionState) {
  if (error) {
    // Send failed
    FUERTE_LOG_CALLBACKS << "asyncWriteCallback: error " << error.message() << std::endl;
    FUERTE_LOG_ERROR << error.message() << std::endl;

    // Item has failed, remove from message store
    _messageStore.removeByID(item->_messageID);

    // let user know that this request caused the error
    item->_onError(errorToInt(ErrorCondition::VstWriteError), std::move(item->_request), nullptr);

    // We'll stop now (and connection state will change in restartConnection).
    _sending = false;

    // Stop current connection and try to restart a new one.
    restartConnection();
  } else {
    // Send succeeded
    FUERTE_LOG_CALLBACKS << "asyncWriteCallback: send succeeded, " << transferred << " bytes transferred" << std::endl;

    // request is written we no longer data for that
    item->resetSendData();

    // Continue with next request (if any)
    FUERTE_LOG_CALLBACKS << "asyncWriteCallback: send next request (if any)" << std::endl;
    sendNextRequest(initialConnectionState);
  }
}
}}}}
