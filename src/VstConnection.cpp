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

  FUERTE_LOG_VSTTRACE << "sendRequest (async): before Lock" << std::endl;

  //start Write may be only entered once!
  bool doWrite;
  {
    Lock lockQueue(_sendQueueMutex);
    doWrite = _sendQueue.empty();
    _sendQueue.push_back(item);
#if ENABLE_FUERTE_LOG_CALLBACKS < 0
    FUERTE_LOG_DEBUG << "queue request" << std::endl;
#endif
    FUERTE_LOG_CALLBACKS << "q";
  }

  FUERTE_LOG_VSTTRACE << "sendRequest (async): before doWrite{..}" << std::endl;

  if (doWrite) {
    // this allows sendRequest to return immediately and
    // not to block until all writing is done
    if(_connected){
      FUERTE_LOG_VSTTRACE << "queue write" << std::endl;
      FUERTE_LOG_VSTTRACE << "messageid: " << item->_request->messageID << " path: " << item->_request->header.path.get() << std::endl;
      //auto self = shared_from_this();
      //_ioService->dispatch( [this,self](){ startWrite(); } );
      startWrite();

      bool alreadyReading = _reading.exchange(true);
      if (!alreadyReading){
        FUERTE_LOG_TRACE << "starting new read" << std::endl;
        startRead();
      } else {
        FUERTE_LOG_TRACE << "NOT starting new read" << std::endl;
      }
    } else {
          FUERTE_LOG_VSTTRACE << "sendRequest (async): not connected" << std::endl;
    }
  }
  FUERTE_LOG_VSTTRACE << "sendRequest (async) done" << std::endl;
  return item->_messageID;
}

std::size_t VstConnection::requestsLeft(){
  // this function does not return the exact size (both mutexes would be
  // required to be locked at the same time) but as it is used to decide
  // if another run is called or not this should not be critical.
  std::size_t left = 0;
  {
    Lock sendQueueLock(_sendQueueMutex);
    left += _sendQueue.size();
  }

  {
    Lock mapLock(_mapMutex);
    left += _messageMap.size();
  }
  return left;
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
    if(!_asioLoop->_running){
      FUERTE_LOG_VSTTRACE << "sendRequest (sync): before run" << std::endl;
      _asioLoop->run_ready();
      //arangodb::fuerte::run(); // run
      FUERTE_LOG_VSTTRACE << "sendRequest (sync): after run" << std::endl;
    }
    FUERTE_LOG_VSTTRACE << "sendRequest (sync): before wait" << std::endl;
    conditionVar.wait(lock, [&]{ return done; });
  }
  FUERTE_LOG_VSTTRACE << "sendRequest (sync): done" << std::endl;
  return std::move(rv);
}


VstConnection::VstConnection(ConnectionConfiguration const& configuration)
    : _vstVersion(VST1_0)
    , _asioLoop(getProvider().getAsioLoop())
    , _messageID(0)
    , _ioService(_asioLoop->getIoService())
    , _socket(nullptr)
    , _context(bs::context::method::sslv23)
    , _sslSocket(nullptr)
    , _connected(false)
    , _pleaseStop(false)
    , _reading(false)
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

void VstConnection::initSocket(){
  FUERTE_LOG_CALLBACKS << "begin init" << std::endl;
  _socket.reset(new bt::socket(*_ioService));
  _sslSocket.reset(new bs::stream<bt::socket&>(*_socket, _context));
  _pleaseStop = false;
  auto endpoints = _endpoints; //copy as connect modifies iterator
  startConnect(endpoints);
}

void VstConnection::shutdownSocket(){
  std::unique_lock<std::mutex> queueLock(_sendQueueMutex, std::defer_lock);
  std::unique_lock<std::mutex> mapLock(_mapMutex, std::defer_lock);
  std::lock(queueLock,mapLock);

  FUERTE_LOG_CALLBACKS << "begin shutdown socket" << std::endl;

  _deadline.cancel();
  BoostEC error;
  _sslSocket->shutdown(error);
  _socket->shutdown(bt::socket::shutdown_both,error);
  _socket->close(error);
  _sslSocket = nullptr;
  _socket = nullptr;
}

void VstConnection::shutdownConnection(){
  // this function must be used in handlers
  bool alreadyStopping = _pleaseStop.exchange(true);
  _reading = false;
  if (alreadyStopping){
    return;
  }

  FUERTE_LOG_CALLBACKS << "begin shutdown connection" << std::endl;
  shutdownSocket();

  Lock mapLock(_mapMutex);
  for(auto& item : _messageMap){
    item.second->_onError(errorToInt(ErrorCondition::VstCanceldDuringReset)
                         ,std::move(item.second->_request)
                         ,nullptr);
  }
  _messageMap.clear();

}

void VstConnection::restartConnection(){
  FUERTE_LOG_CALLBACKS << "restart" << std::endl;
  shutdownConnection();
  initSocket();
}

// ASIO CONNECT

// try to open the socket connection to the first endpoint.
void VstConnection::startConnect(bt::resolver::iterator endpointItr){
  if (endpointItr != boost::asio::ip::tcp::resolver::iterator()){
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
void VstConnection::asyncConnectCallback(BoostEC const& error, bt::resolver::iterator endpointItr){
  if(!error){
    FUERTE_LOG_CALLBACKS << "connected" << std::endl;
    //if success - start async handshake
    if(_configuration._ssl){
      FUERTE_LOG_CALLBACKS << "call start startHandshake" << std::endl;
      startHandshake();
      return;
    } else {
      FUERTE_LOG_CALLBACKS << "call finish init" << std::endl;
      finishInitialization();
      return;
    }
  } else {
    FUERTE_LOG_ERROR << error.message() << std::endl;
    shutdownConnection();
    if(endpointItr == bt::resolver::iterator()){
      FUERTE_LOG_CALLBACKS << "no further endpoint" << std::endl;
    }
    throw std::runtime_error("unable to connect -- " + error.message() );
    return;
  }
  throw std::logic_error("you should not end up here!!!");
}

// socket connection is up (with optional SSL), now initiate the VST protocol.
void VstConnection::finishInitialization(){
  FUERTE_LOG_CALLBACKS << "finish initialization" << std::endl;
  {
    _connected = true;
  }

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
                      startWrite(true); // there might be no requests enqueued
                      bool alreadyReading = _reading.exchange(true);
                      if (!alreadyReading){
                        startRead();
                      }
                    }
                  }
                 );
}

void VstConnection::startHandshake(){
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

void VstConnection::startRead(){
  FUERTE_LOG_VSTTRACE << "startRead" << std::endl;
  FUERTE_LOG_CALLBACKS << "-";
  if (_pleaseStop) {
    FUERTE_LOG_VSTTRACE << "startRead: pleaseStop" << std::endl;
    return;
  }

  {
    std::unique_lock<std::mutex> queueLock(_sendQueueMutex, std::defer_lock);
    std::unique_lock<std::mutex> mapLock(_mapMutex, std::defer_lock);
    std::lock(queueLock,mapLock);
    if(_messageMap.empty() && _sendQueue.empty()){
      _reading = false;
      FUERTE_LOG_VSTTRACE << "returning from read loop";
      FUERTE_LOG_CALLBACKS <<  std::endl;
      return;
    }
  }

  // gets data from network and fill
  _deadline.expires_from_now(boost::posix_time::seconds(30));
  FUERTE_LOG_CALLBACKS << "r";
#if ENABLE_FUERTE_LOG_CALLBACKS > 0
  std::cout << "_messageMap = " << mapToKeys(_messageMap) << std::endl;
  std::cout.flush();
#endif
  auto self = shared_from_this();
  ba::async_read(*_socket
                ,_receiveBuffer
                ,ba::transfer_at_least(4)
                ,[this,self](const boost::system::error_code& error, std::size_t transferred){
                   this->asyncReadCallback(error, transferred);
                 }
                );

  FUERTE_LOG_VSTTRACE << "startRead: done" << std::endl;
}

// asyncReadCallback is called when startRead is resulting in some data.
void VstConnection::asyncReadCallback(const boost::system::error_code& error, std::size_t transferred){
  FUERTE_LOG_CALLBACKS << "asyncReadCallback begin" ;
  if (error){
    FUERTE_LOG_CALLBACKS << "Error while reading form socket";
    FUERTE_LOG_ERROR << error.message() << std::endl;
    restartConnection();
  }

  FUERTE_LOG_CALLBACKS << "R(" << transferred << ")" ;

  if(_pleaseStop) {
    FUERTE_LOG_CALLBACKS << "asyncReadCallback: pleaseStop" ;
    return;
  }

  // Inspect the data we've received so far.
  auto receivedBuf = _receiveBuffer.data(); //no copy
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
  startRead();
}

// Process the given incoming chunk.
void VstConnection::processChunk(ChunkHeader &chunk) {
  auto msgID = chunk.messageID();
  FUERTE_LOG_VSTTRACE << "processChunk: messageID=" << msgID << std::endl;

  // Find requestItem for this chunk.  
  ::std::map<MessageID,std::shared_ptr<RequestItem>>::iterator found;
  {
    Lock lock(_mapMutex);
    found = _messageMap.find(chunk._messageID);
    if (found == _messageMap.end()) {
      FUERTE_LOG_ERROR << "got chunk with unknown message ID: " << msgID << std::endl;
      return;
    }
  }

  // We've found the matching RequestItem.
  FUERTE_LOG_VSTTRACE << "processChunk: found RequestItem: addChunk" << std::endl;
  auto item = found->second;
  item->addChunk(chunk);

  // Try to assembly chunks in RequestItem to complete response.
  FUERTE_LOG_VSTTRACE << "processChunk: found RequestItem: assemble" << std::endl;
  auto completeBuffer = item->assemble();
  if (completeBuffer) {
    // Message is complete 
    // Remove message from store 
    {
      Lock lock(_mapMutex);
      _messageMap.erase(item->_messageID);
    }

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
  itemCursor += messageHeaderLength;
  itemLength -= messageHeaderLength;

  auto response = std::unique_ptr<Response>(new Response(std::move(messageHeader)));
  response->messageID = item._messageID;
  response->setPayload(std::move(*responseBuffer));

  return response;
}

// writes data from task queue to network using boost::asio::async_write
void VstConnection::startWrite(bool possiblyEmpty) {
  FUERTE_LOG_CALLBACKS << "startWrite" << std::endl;
  FUERTE_LOG_TRACE << "+" ;
  if (_pleaseStop) {
    FUERTE_LOG_CALLBACKS << "startWrite: pleaseStop" << std::endl;
    return;
  }

  std::shared_ptr<RequestItem> next;
  {
    Lock sendQueueLock(_sendQueueMutex);
    if(_sendQueue.empty()){
      assert(possiblyEmpty);
      FUERTE_LOG_CALLBACKS << "startWrite: sendQueue empty" << std::endl;
      return;
    }
    next = _sendQueue.front();
  }

  {
    Lock mapLock(_mapMutex);
    _messageMap.emplace(next->_messageID, next);
  }

  FUERTE_LOG_CALLBACKS << "startWrite: preparing to send next" << std::endl;

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
    [this, self, next](BoostEC const& error, std::size_t transferred) {
      this->asyncWriteCallback(error, transferred, next);
    });

  FUERTE_LOG_CALLBACKS << "startWrite: done" << std::endl;
}

// callback of async_write function that is called in startWrite.
void VstConnection::asyncWriteCallback(BoostEC const& error, std::size_t transferred, RequestItemSP item){
  FUERTE_LOG_CALLBACKS << "asyncWriteCallback" << std::endl;

  if (error){
    FUERTE_LOG_CALLBACKS << "asyncWriteCallback: error " << error.message() << std::endl;
    FUERTE_LOG_ERROR << error.message() << std::endl;
    _pleaseStop = true; //stop reading as well
    {
      Lock lock(_mapMutex);
      _messageMap.erase(item->_messageID);
    }

    //let user know that this request caused the error
    item->_onError(errorToInt(ErrorCondition::VstWriteError),std::move(item->_request),nullptr);

    restartConnection();

    // pop at the very end of error handling so nothing else
    // gets queued in the io_service
    Lock sendQueueLock(_sendQueueMutex);
    _sendQueue.pop_front();
    return;
  }

  FUERTE_LOG_CALLBACKS << "asyncWriteCallback: send succeeded, " << transferred << " bytes transferred" << std::endl;

  item->resetSendData(); //request is written we no longer data for that
  // everything is ok
  // remove item when work is done;
  // so the queue does not get empty in between which could
  // trigger another parallel write that is not allowed
  // the caller of async_write has to make sure that there
  // are no parallel calls
  {
    Lock sendQueueLock(_sendQueueMutex);
    _sendQueue.pop_front();
    if(_sendQueue.empty()) { 
      FUERTE_LOG_CALLBACKS << "asyncWriteCallback: sendQueue is empty" << std::endl;
      return; 
    }
  }

  FUERTE_LOG_CALLBACKS << "asyncWriteCallback: send next request" << std::endl;

  startWrite();
  //auto self = shared_from_this();
  //_ioService->dispatch( [self](){ self->startWrite(); });
}
}}}}
