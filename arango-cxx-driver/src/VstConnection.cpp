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
/// @author Frank Celler
/// @author Jan Uhde
////////////////////////////////////////////////////////////////////////////////

#include "VstConnection.h"
#include <condition_variable>
#include <fuerte/loop.h>
#include <fuerte/vst.h>
#include <fuerte/message.h>
#include "asio/SocketUnixDomain.h"

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

MessageID VstConnection::sendRequest(std::unique_ptr<Request> request,
                                 OnErrorCallback onError,
                                 OnSuccessCallback onSuccess){

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
  }

  if(doWrite){
    auto self = shared_from_this();
    // this allows sendRequest to return immediatly and
    // not to block until all writing is done
    _ioService->post( [this,self](){ startWrite(); } );
  }
  return item->_messageId;
}

std::unique_ptr<Response> VstConnection::sendRequest(RequestUP request){
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
      mutex.unlock();
      conditionVar.notify_one();
    };

    auto onSuccess  = [&](RequestUP request, ResponseUP response){
      rv = std::move(response);
      done = true;
      mutex.unlock();
      conditionVar.notify_one();
    };

    sendRequest(std::move(request),onError,onSuccess);
    LoopProvider::getProvider().pollAsio(); //REVIEW

    {
      //wait for handler to call notify_one
      std::unique_lock<std::mutex> lock(mutex);
      conditionVar.wait(lock, [&]{ return done; });
    }
    return std::move(rv);
}


VstConnection::VstConnection(ConnectionConfiguration configuration)
    : _asioLoop(LoopProvider::getProvider().getAsioLoop())
    , _ioService(reinterpret_cast<ba::io_service*>(LoopProvider::getProvider().getAsioIoService()))
    , _socket(nullptr)
    , _context(bs::context::method::sslv23)
    , _sslSocket(nullptr)
    , _pleaseStop(false)
    , _stopped(false)
    , _configuration(configuration)
    , _deadline(*_ioService)
{
    bt::resolver resolver(*_ioService);
    _endpoints = resolver.resolve({configuration._host,configuration._port});
    // check if endpoints are empty - and throw TODO
    initSocket(_endpoints);
}

void VstConnection::initSocket(bt::resolver::iterator endpoints){
  _pleaseStop = false;
  _socket.reset(new bt::socket(*_ioService));
  _sslSocket.reset(new bs::stream<bt::socket&>(*_socket, _context));
  _stopped = false;
  startConnect(endpoints);
}

void VstConnection::shutdownSocket(){
  _pleaseStop = true;
  _deadline.cancel();
  while(true){
    if(!_handlercount){
      break;
    }
  }
  _sslSocket->shutdown();
  _socket->shutdown(bt::socket::shutdown_both);

  //delete requests in flight
  //reset streams
  _stopped = true; // create a timer that periodically checks this variable
                   // and reinitalises the loop - this way we avoid deeper stacks

  //_socket->close(); will be closed on deconstruction
  _socket->close();
}

void VstConnection::shutdownConnection(){
  shutdownSocket();

  Lock mapLock(_mapMutex);
  for(auto& item : _messageMap){
    // answer requests TODO
    item.second->_onError(100,std::move(item.second->_request),nullptr);
  }
  _messageMap.clear();

}

void VstConnection::restartConnection(){
  shutdownConnection();
  initSocket(_endpoints);
}

void VstConnection::startConnect(bt::resolver::iterator endpointItr){
  using namespace std::placeholders;
  if (endpointItr != boost::asio::ip::tcp::resolver::iterator()){
    std::cout << "Trying " << endpointItr->endpoint() << "...\n";

    // Set a deadline for the connect operation.
    _deadline.expires_from_now(boost::posix_time::seconds(60));

    // Start the asynchronous connect operation.
    ++_handlercount;
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
  assert(_handlercount);
  --_handlercount;
  if(!error){
    //if success - start async handshake
    if(_configuration._ssl){
      startHandshake();
    } else {
      startRead();
    }
  } else {
    //if error - start connect with next endpoint
    //startConnect(++endpointItr);
    //::boost::asio::async_connect iterates the endpoint list
    //so we need to check for the end endpoint

    //if (endpointItr != boost::asio::ip::tcp::resolver::iterator()){
    // There are no more endpoints to try. Shut down the client.
    shutdownSocket();
    return;
    //}
  }
}

void VstConnection::startHandshake(){

}

void VstConnection::handleHandshake(){
  assert(_handlercount);
  --_handlercount;
  startRead();
}

void VstConnection::startRead(){
  if (_pleaseStop) {
    return;
  }

  // gets data from network and fill
  _deadline.expires_from_now(boost::posix_time::seconds(30));

  auto self = shared_from_this();
  ++_handlercount;
  ba::async_read(*_socket
                ,_receiveBuffer
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

void VstConnection::handleRead(const boost::system::error_code& error, std::size_t transferred){
  assert(_handlercount);
  --_handlercount;
  if(_pleaseStop) {
    return;
  }

  boost::asio::const_buffer received = _receiveBuffer.data(); //no copy
  std::size_t size = boost::asio::buffer_size(received);
  assert(transferred == size);
  auto pair = bufferToPointerAndSize(received); //get write access
  auto cursor = pair.first;
  auto length = pair.second;

  _receiveBuffer.consume(length); //remove chung form input
  if (!vst::isChunkComplete(pair.first, pair.second)){
    startRead();
    return;
  }

  bool processData = false; // will be set to true if we have a complete message

  auto vstChunkHeader = vst::readChunkHeaderV1_0(pair.first);
  cursor += vstChunkHeader.chunkHeaderLength;
  length -= vstChunkHeader.chunkHeaderLength;

  ::std::map<MessageID,std::shared_ptr<RequestItem>>::iterator found;
  {
    Lock lock(_mapMutex);
    found = _messageMap.find(vstChunkHeader.messageID);
    if (found == _messageMap.end()) {
      throw std::logic_error("got message with not matching request");
    }
  }

  RequestItem& item = *(found->second);
  item._responseBuffer.append(cursor,length);

  if(vstChunkHeader.isSingle){ //we got a single chunk containing the complete message
    processData = true;
  } else if (!vstChunkHeader.isFirst){ //there is chunk that continues a message
    item._responseLength = vstChunkHeader.totalMessageLength;
    item._responseChunks = vstChunkHeader.chunk;
    item._responseChunk = 1;
  } else { //the chunk stats a multipart message
    item._responseChunk++;
    assert(item._responseChunk == vstChunkHeader.chunk); //V1.0
    if(item._responseChunks == vstChunkHeader.chunk){ //last chunk reached
      processData = true;
    }
  }

  startRead(); //start next read - code below might run in parallel to new read

  if(processData){
    cursor = item._responseBuffer.data();
    length = item._responseBuffer.byteSize();
    std::size_t messageHeaderLength;
    MessageHeader messageHeader = validateAndExtractMessageHeader(cursor, length, messageHeaderLength);
    cursor += messageHeaderLength;
    length += messageHeaderLength;

    auto response = std::unique_ptr<Response>(new Response());
    //add header and strings!!!!
    throw std::logic_error("implement me");

    // finally add payload

    // avoiding the copy would imply that we mess with offsets
    // if feels like Velocypack could gain some options like
    // adding some offset for a buffer that way the already
    // allocated memory could be reused.
    if(messageHeader.contentType == ContentType::VPack){
      auto numPayloads = vst::validateAndCount(cursor,length);
      VBuffer buffer;
      buffer.append(cursor,length); //we should avoid this copy FIXME
      response->addVPack(std::move(buffer)); //ASK jan
    }
    // call callback
    item._onSuccess(std::move(item._request),std::move(response));
  }
}

void VstConnection::startWrite(){
  if (_pleaseStop) {
    return;
  }

  assert(!_sendQueue.empty()); //the queue can not be empty
  // no lock required here to accesse the first element as it can not change
  // front will be only modified at the end of this function
  auto next = _sendQueue.front();
  {
    Lock mapLock(_mapMutex);
    _messageMap.emplace(next->_messageId,next);
  }

  // make sure we are connected and handshake has been done
  auto self = shared_from_this();
  auto data = *next->_requestBuffer;
  ++_handlercount;
  ba::async_write(*_socket
                 ,ba::buffer(data.data(),data.byteSize())
                 ,[this,self,next,data](BoostEC const& error, std::size_t transferred){
                    this->handleWrite(error,transferred, next);
                  }
                 );
}

void VstConnection::handleWrite(BoostEC const& error, std::size_t transferred, RequestItemSP item){
  assert(_handlercount);
  --_handlercount;

  if (error){
    _pleaseStop = true; //stop reading as well
    {
      Lock lock(_mapMutex);
      _messageMap.erase(item->_messageId);
    }

    //let user know that this request caused the error
    item->_onError(100,std::move(item->_request),nullptr);

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
