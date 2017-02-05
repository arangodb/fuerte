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

#include "FuerteLogger.h"
#include "VstConnection.h"
#include <condition_variable>
#include <fuerte/loop.h>
#include <fuerte/vst.h>
#include <fuerte/message.h>

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
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
                                    ,OnSuccessCallback onSuccess){

  //check if id is already used and fail
  request->messageid = ++_messageId;
  auto item = std::make_shared<RequestItem>();

  item->_messageId = _messageId;
  item->_onError = onError;
  item->_onSuccess = onSuccess;
  item->_requestBuffer = vst::toNetwork(*request);
  item->_request = std::move(request);

  //start Write may be only entered once!
  bool doWrite;
  {
    Lock lockQueue(_sendQueueMutex);
    doWrite = _sendQueue.empty();
    _sendQueue.push_back(item);
    FUERTE_LOG_DEBUG << "request queued" << std::endl;
  }

  if(doWrite){
    // this allows sendRequest to return immediatly and
    // not to block until all writing is done
    if(_connected){
      FUERTE_LOG_VSTTRACE << "queue write" << std::endl;
      auto self = shared_from_this();
      //_ioService->dispatch( [this,self](){ startWrite(); } );
      startWrite();

      bool alreadyReading = _reading.exchange(true);
      if (!alreadyReading){
        FUERTE_LOG_TRACE << "starting new read" << std::endl;
        startRead();
      } else {
        FUERTE_LOG_TRACE << "NOT starting new read" << std::endl;
      }
    }
  }
  return item->_messageId;
}

std::unique_ptr<Response> VstConnection::sendRequest(RequestUP request){
  FUERTE_LOG_VSTTRACE << "start sync request" << std::endl;
  // TODO - we expect the loop to be runniung even for sync requests
  // maybe we could call poll on the ioservice in this function if
  // it is not running.
  std::mutex mutex;
  std::condition_variable conditionVar;
  bool done = false;

  auto rv = std::unique_ptr<Response>(nullptr);
  auto onError  = [&](::arangodb::fuerte::v1::Error error, RequestUP request, ResponseUP response){
    rv = std::move(response);
    done = true;
    conditionVar.notify_one();
  };

  auto onSuccess  = [&](RequestUP request, ResponseUP response){
    rv = std::move(response);
    done = true;
    conditionVar.notify_one();
  };

  {
    std::unique_lock<std::mutex> lock(mutex);
    sendRequest(std::move(request),onError,onSuccess);
    if(!_asioLoop->_running){
      arangodb::fuerte::run();
    }
    conditionVar.wait(lock, [&]{ return done; });
  }
  return std::move(rv);
}


VstConnection::VstConnection(ConnectionConfiguration const& configuration)
    : _asioLoop(getProvider().getAsioLoop())
    , _ioService(_asioLoop->getIoService())
    , _socket(nullptr)
    , _context(bs::context::method::sslv23)
    , _sslSocket(nullptr)
    , _connected(false)
    , _pleaseStop(false)
    , _reading(false)
    , _configuration(configuration)
    , _deadline(*_ioService)
    , _vstVersionID(1)
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
  FUERTE_LOG_DEBUG << "begin init" << std::endl;
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

  FUERTE_LOG_DEBUG << "begin shutdown socket" << std::endl;

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

  FUERTE_LOG_DEBUG << "begin shutdown connection" << std::endl;
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
  FUERTE_LOG_DEBUG << "restart" << std::endl;
  shutdownConnection();
  initSocket();
}

// ASIO CONNECT

void VstConnection::startConnect(bt::resolver::iterator endpointItr){
  if (endpointItr != boost::asio::ip::tcp::resolver::iterator()){
    FUERTE_LOG_DEBUG << "trying to connect to: " << endpointItr->endpoint() << "..." << std::endl;

    // Set a deadline for the connect operation.
    _deadline.expires_from_now(boost::posix_time::seconds(60));

    // Start the asynchronous connect operation.
    auto self = shared_from_this();
    ba::async_connect(*_socket
                     ,endpointItr
                     ,[this,self](BoostEC const& error, bt::resolver::iterator endpointItr){
                        handleConnect(error,endpointItr);
                      }
                     );
  }
}

void VstConnection::handleConnect(BoostEC const& error, bt::resolver::iterator endpointItr){
  if(!error){
    FUERTE_LOG_DEBUG << "connected" << std::endl;
    //if success - start async handshake
    if(_configuration._ssl){
      FUERTE_LOG_DEBUG << "call start startHandshake" << std::endl;
      startHandshake();
      return;
    } else {
      FUERTE_LOG_DEBUG << "call finish init" << std::endl;
      finishInitialization();
      return;
    }
  } else {
    FUERTE_LOG_ERROR << error.message() << std::endl;
    shutdownConnection();
    if(endpointItr == bt::resolver::iterator()){
      FUERTE_LOG_DEBUG << "no further endpoint" << std::endl;
    }
    throw std::runtime_error("unable to connect -- " + error.message() );
    return;
  }
  throw std::logic_error("you should not end up here!!!");
}

void VstConnection::finishInitialization(){
  FUERTE_LOG_DEBUG << "finish initialization" << std::endl;
  {
    _connected = true;
  }
  startWrite(true); // there might be no requests enqueued
  bool alreadyReading = _reading.exchange(true);
  if (!alreadyReading){
    startRead();
  }
}

void VstConnection::startHandshake(){
  if(!_configuration._ssl){
    finishInitialization();
  }
  FUERTE_LOG_DEBUG << "starting ssl handshake " << std::endl;
  auto self = shared_from_this();
  _sslSocket->async_handshake(bs::stream_base::client
                             ,[this,self](BoostEC const& error){
                               if(error){
                                 shutdownSocket();
                                 FUERTE_LOG_ERROR << error.message() << std::endl;
                                 throw std::runtime_error("unable to perform ssl handshake");
                               }
                               FUERTE_LOG_DEBUG << "ssl handshake done" << std::endl ;
                               finishInitialization();
                              }
                            );

}

// READING WRITING / NORMAL OPERATIONS  ///////////////////////////////////////

void VstConnection::startRead(){
  FUERTE_LOG_DEBUG << "-";
  if (_pleaseStop) {
    return;
  }

  {
    std::unique_lock<std::mutex> queueLock(_sendQueueMutex, std::defer_lock);
    std::unique_lock<std::mutex> mapLock(_mapMutex, std::defer_lock);
    std::lock(queueLock,mapLock);
    if(_messageMap.empty() && _sendQueue.empty()){
      _reading = false;
      FUERTE_LOG_VSTTRACE << "returning from read loop";
      FUERTE_LOG_DEBUG <<  std::endl;
      return;
    }
  }

  // gets data from network and fill
  _deadline.expires_from_now(boost::posix_time::seconds(30));
  FUERTE_LOG_DEBUG << "r";
  auto self = shared_from_this();
  ba::async_read(*_socket
                ,_receiveBuffer
                ,ba::transfer_at_least(4)
                ,[this,self](const boost::system::error_code& error, std::size_t transferred){
                   handleRead(error,transferred);
                 }
                );
}

// helper for turning a asio::const_buffer into a accessible form
template<typename T = uint8_t>
std::pair<T const*, std::size_t> bufferToPointerAndSize(boost::asio::const_buffer& buffer){
  return std::make_pair( boost::asio::buffer_cast<T const*>(buffer)
                       , boost::asio::buffer_size(buffer));
}

std::tuple<bool,std::shared_ptr<RequestItem>,std::size_t> VstConnection::processChunk(uint8_t const * cursor, std::size_t length){
  FUERTE_LOG_VSTTRACE << "\n\n\nENTER PROCESS CHUNK" << std::endl;
  auto vstChunkHeader = vst::readChunkHeaderV1_0(cursor);
  //peek next chunk
  bool nextChunkAvailable = false;
  if(length > vstChunkHeader._chunkLength + sizeof(ChunkHeader::_chunkLength)){
    FUERTE_LOG_VSTTRACE << "processing chunk with messageid: " << vstChunkHeader._messageID << std::endl;
    FUERTE_LOG_VSTTRACE << "peeking into next chunk!" << std::endl;
    nextChunkAvailable = vst::isChunkComplete(cursor + vstChunkHeader._chunkLength
                                             ,length - vstChunkHeader._chunkLength);
    FUERTE_LOG_VSTTRACE << "next availalbe: " << nextChunkAvailable << std::endl;
  }

  cursor += vstChunkHeader._chunkHeaderLength;
  length -= vstChunkHeader._chunkHeaderLength;


  FUERTE_LOG_VSTTRACE << "ChunkHeaderLenth: " << vstChunkHeader._chunkHeaderLength << std::endl;

  ::std::map<MessageID,std::shared_ptr<RequestItem>>::iterator found;
  {
    Lock lock(_mapMutex);
    found = _messageMap.find(vstChunkHeader._messageID);
    if (found == _messageMap.end()) {
      throw std::logic_error("got message with no matching request");
    }
  }


  RequestItemSP item = found->second;

  FUERTE_LOG_VSTTRACE << "appending to item with length: " << vstChunkHeader._chunkPayloadLength << std::endl;
  // copy payload to buffer
  item->_responseBuffer.append(cursor,vstChunkHeader._chunkPayloadLength);

  cursor += vstChunkHeader._chunkPayloadLength;
  length -= vstChunkHeader._chunkPayloadLength;

  FUERTE_LOG_VSTTRACE << "next chunk availalbe: " << std::boolalpha << nextChunkAvailable  << std::endl;

  if(vstChunkHeader._isSingle){ //we got a single chunk containing the complete message
    FUERTE_LOG_VSTTRACE << "adding single chunk " << std::endl;
    item->_responseBuffer.resetTo(vstChunkHeader._chunkPayloadLength);
    return std::tuple<bool,RequestItemSP,std::size_t>(nextChunkAvailable, std::move(item), vstChunkHeader._chunkLength);
  } else if (!vstChunkHeader._isFirst){
    //there is chunk that continues a message
    item->_responseChunk++;
    if(item->_responseChunks == vstChunkHeader._numberOfChunks){ //last chunk reached
      FUERTE_LOG_VSTTRACE << "adding multi chunk " << std::endl;
      // TODO TODO TODO TODO should multichunk not be working start looking here!!!!!!!
      // the VPackBuffer is unable to track how much has been written to it. Maybe this is
      // fixed when you read this.
      //assert(item->_responseBuffer.length() == item->_responseLength);
      item->_responseBuffer.resetTo(item->_responseLength);
      return std::tuple<bool,RequestItemSP,std::size_t>(nextChunkAvailable, std::move(item),vstChunkHeader._chunkLength);
    }
    FUERTE_LOG_VSTTRACE << "multi chunk incomplte" << std::endl;
  } else {
    //the chunk stats a multipart message
    item->_responseLength = vstChunkHeader._totalMessageLength;
    item->_responseChunks = vstChunkHeader._numberOfChunks;
    item->_responseChunk = 1;
    FUERTE_LOG_VSTTRACE << "starting multi chunk" << std::endl;
  }

  return std::tuple<bool,RequestItemSP,std::size_t>(nextChunkAvailable, nullptr, vstChunkHeader._chunkLength);
}

void VstConnection::processCompleteItem(std::shared_ptr<RequestItem>&& itempointer){
  RequestItem& item = *itempointer;
  FUERTE_LOG_VSTTRACE << "completing item with messageid: " << item._messageId << std::endl;
  auto itemCursor = item._responseBuffer.data();
  auto itemLength = item._responseBuffer.byteSize();
  std::size_t messageHeaderLength;
  MessageHeader messageHeader = validateAndExtractMessageHeader(_vstVersionID, itemCursor, itemLength, messageHeaderLength);
  itemCursor += messageHeaderLength;
  itemLength -= messageHeaderLength;

  auto response = std::unique_ptr<Response>(new Response(std::move(messageHeader)));
  // finally add payload

  // avoiding the copy would imply that we mess with offsets
  // if feels like Velocypack could gain some options like
  // adding some offset for a buffer that way the already
  // allocated memory could be reused.
  if(messageHeader.contentType == ContentType::VPack){
    auto numPayloads = vst::validateAndCount(itemCursor,itemLength);
    FUERTE_LOG_VSTTRACE << "number of slices: " << numPayloads << std::endl;
    VBuffer buffer;
    buffer.append(itemCursor,itemLength); //we should avoid this copy FIXME
    buffer.resetTo(itemLength);
    auto slice = VSlice(itemCursor);
    FUERTE_LOG_VSTTRACE << slice.toJson() << " , " << slice.byteSize() << std::endl;
    FUERTE_LOG_VSTTRACE << "buffer size" << " , " << buffer.size() << std::endl;
    response->addVPack(std::move(buffer)); //ASK jan
    FUERTE_LOG_VSTTRACE << "payload size" << " , " << response->payload().second << std::endl;
  } else {
    response->addBinary(itemCursor,itemLength);
  }
  // call callback
  item._onSuccess(std::move(item._request),std::move(response));
  {
    Lock mapLock(_mapMutex);
    _messageMap.erase(item._messageId);
  }
}

void VstConnection::handleRead(const boost::system::error_code& error, std::size_t transferred){
  if (error){
    FUERTE_LOG_DEBUG << "Error while reading form socket";
    FUERTE_LOG_ERROR << error.message() << std::endl;
    restartConnection();
  }

  FUERTE_LOG_DEBUG << "R(" << transferred << ")" ;

  if(_pleaseStop) {
    return;
  }

  //THIS WIL MAKE THE CODE FAIL WHEN RECONNECTING
  //if (!transferred) {
  // throw std::logic_error("handler called without receiving data");
  //}

  boost::asio::const_buffer received = _receiveBuffer.data(); //no copy
  std::size_t size = boost::asio::buffer_size(received);

  //assert(transferred == size); // could be longer because we do not take everyting form the buffer
  auto pair = bufferToPointerAndSize<uint8_t>(received); //get write access
  if (!vst::isChunkComplete(pair.first, pair.second)){
    FUERTE_LOG_DEBUG << "no complete chunk continue reading" << std::endl;
    startRead();
    return;
  }

  uint8_t const* cursor = pair.first;
  auto length = pair.second;
  std::size_t consumed = 0;
  std::vector<std::shared_ptr<RequestItem>> items;

  { // limit socpe of vars
    RequestItemSP item = nullptr; // id is given only when a chunk is complete
    bool processMoreChunks = true;
    std::size_t consume;

    while(processMoreChunks){
      std::tie(processMoreChunks,item,consume) = processChunk(cursor, length);
      consumed += consume;
      cursor += consume;
      length -= consume;
      if(item){
        items.push_back(std::move(item));
      }
    }

    _receiveBuffer.consume(consumed); //remove chunk from input
  }

  /// end new function

  if(!_asioLoop->_singleRunMode){
    startRead(); //start next read - code below might run in parallel to new read
  }

  if(!items.empty()){
    for(auto itempointer : items){
      processCompleteItem(std::move(itempointer)); //maybe as ref or plain pointer?!
    }
  }

  if(_asioLoop->_singleRunMode){
    startRead();
  }
}

void VstConnection::startWrite(bool possiblyEmpty){
  FUERTE_LOG_TRACE << "+" ;
  if (_pleaseStop) {
    return;
  }

  std::shared_ptr<RequestItem> next;
  {
    Lock sendQueueLock(_sendQueueMutex);
    if(_sendQueue.empty()){
      assert(possiblyEmpty);
      return;
    }
    next = _sendQueue.front();
  }

  {
    Lock mapLock(_mapMutex);
    _messageMap.emplace(next->_messageId,next);
  }

  FUERTE_LOG_DEBUG << "s";

  // make sure we are connected and handshake has been done
  auto self = shared_from_this();
  auto data = *next->_requestBuffer;
  FUERTE_LOG_DEBUG << data.byteSize();
  ba::async_write(*_socket
                 ,ba::buffer(data.data(),data.byteSize())
                 ,[this,self,next,data](BoostEC const& error, std::size_t transferred){
                    this->handleWrite(error,transferred, next);
                  }
                 );
}

void VstConnection::handleWrite(BoostEC const& error, std::size_t transferred, RequestItemSP item){
  FUERTE_LOG_DEBUG << "S";

  if (error){
    FUERTE_LOG_ERROR << error.message() << std::endl;
    _pleaseStop = true; //stop reading as well
    {
      Lock lock(_mapMutex);
      _messageMap.erase(item->_messageId);
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
  item->_requestBuffer.reset(); //request is written we no longer need the buffer
  //everything is ok
  // remove item when work is done;
  // so the queue does not get empty in between wich could
  // trigger another parallel wirte that is not allowed
  // the caller of async_write has to make sure that there
  // are no parallel calls
  {
    Lock sendQueueLock(_sendQueueMutex);
    _sendQueue.pop_front();
    if(_sendQueue.empty()){ return; }
  }
  startWrite();
}
}}}}
