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
#include "helper.h"
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

using namespace std::placeholders;

void VstConnection::sendRequest(std::unique_ptr<Request> request,
                                 OnErrorCallback onError,
                                 OnSuccessCallback onSuccess){

  //check if id is already used and fail
  request->messageid = ++_messageId;
  auto item = std::make_shared<SendItem>();

  item->_messageId = _messageId;
  item->_onError = onError;
  item->_onSuccess = onSuccess;
  item->_request = std::move(request);

  Lock lockQueue(_sendQueueMutex);
  auto doWrite = _sendQueue.empty();
  _sendQueue.push_back(item);
  if(doWrite){
    startWrite();
  }



  //std::pair<std::map<MessageID,SendItem>::iterator,bool> insert;
  //{
  //  Lock lock(_mapMutex);
  //  insert = _messageMap.emplace(_messageId ,std::move(item));
  //}

  //if(!insert.second){
  //  onError(100,std::move(request),nullptr); //errors
  //  return;
  //}
  //MapItem& itemRef = insert.first->second;



  //startWrite(_messageId, *itemRef._request);

}


//client connection
VstConnection::VstConnection(ConnectionConfiguration configuration)
    : _asioLoop(LoopProvider::getProvider().getAsioLoop())
    , _ioService(_asioLoop->getIoService())
    , _socket(nullptr)
    , _context(bs::context::method::sslv23)
    , _sslSocket(nullptr)
    , _pleaseStop(false)
    , _stopped(false)
    , _configuration(configuration)
    , _deadline(*_ioService)
{
    bt::resolver resolver(*_ioService);
    auto endpoint_iterator = resolver.resolve({configuration._host,configuration._port});
    startConnect(endpoint_iterator);
}

void VstConnection::init_socket(){
  _pleaseStop = false;
  _socket.reset(new bt::socket(*_ioService));
  _sslSocket.reset(new bs::stream<bt::socket&>(*_socket, _context));
  _stopped = false;
}

void VstConnection::shutdown_socket(){
  _pleaseStop = true;
  _deadline.cancel();
  while(true){
    if(!_handlercount){
      break;
    }
  }
  _sslSocket->shutdown();
  _socket->shutdown(bt::socket::shutdown_both);
  //_socket->close(); will be closed on deconstruction

  //delete requests in flight
  //reset streams
  _stopped = true; // create a timer that periodically checks this variable
                   // and reinitalises the loop - this way we avoid deeper stacks
  init_socket();   // replace this call with the help of _stopped variable as described above
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
                     ,[this,self](boost::system::error_code const& error, bt::resolver::iterator endpointItr){
                        handleConnect(error,endpointItr);
                      }
                     );
  } else {
    // There are no more endpoints to try. Shut down the client.
    shutdown_socket();
  }
}

void VstConnection::handleConnect(boost::system::error_code const& error, bt::resolver::iterator endpointItr){
  assert(_handlercount);
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
      --_handlercount;
      shutdown_socket();
      return;
    //}
  }
  --_handlercount;
}


void VstConnection::startHandshake(){

}

void VstConnection::handleHandshake(){
  assert(_handlercount);
  startRead();
  --_handlercount;
}

void VstConnection::startRead(){
  if (_pleaseStop) {
    return;
  }

  // gets data from network and fill
  _deadline.expires_from_now(boost::posix_time::seconds(30));

  ++_handlercount;
  auto self = shared_from_this();
  ba::async_read(*_socket
                ,_receiveBuffer
                ,[this,self](const boost::system::error_code& error, std::size_t transferred){
                   handleRead(error,transferred);
                 }
                );
}

void VstConnection::handleRead(const boost::system::error_code& error, std::size_t transferred){
  assert(_handlercount);
  // get data form receive buffer
  // remove data from buffer
  startRead(); //start next read -- no locking required
  // create message from data
  // call callback
  --_handlercount;
}

void VstConnection::startWrite(){
  if (_pleaseStop) {
    return;
  }

  while(true){
    //no lock required here
    auto next = _sendQueue.front();
    {
      Lock mapLock(_mapMutex);
      _messageMap.emplace(next->_messageId,next);
    }

    // make sure we are connected and handshake has been done
    auto self = shared_from_this();
    auto data = vst::toNetwork(*next->_request);
    ++_handlercount;
    ba::async_write(*_socket
                   ,ba::buffer(data->data(),data->byteSize())
                   ,[this,self,next,data](const boost::system::error_code& error, std::size_t transferred){
                      handleWrite(error,transferred, next);
                    }
                   );

    // remove item when work is done;
    // so the queue does not get empty in between wich could
    // trigger another parallel wirte that is not allowed
    // the caller of async_write has to make sure that there
    // are no parallel calls
    Lock sendQueueLock(_sendQueueMutex);
    _sendQueue.pop_front();
    if(_sendQueue.empty()){
      break;
    }
  }

}

void VstConnection::handleWrite(const boost::system::error_code& error, std::size_t transferred, SendItemSP item){
  assert(_handlercount);
  if (error){
    {
      Lock lock(_mapMutex);
      //remove message form map of messages to receive
      _messageMap.erase(item->_messageId);
    }
    --_handlercount;
    item->_onError(100,std::move(item->_request),nullptr);
    // connection error?
    // shutdown
    // restart
    return;
  }
  //everything is ok
  --_handlercount;
}
}}}}
