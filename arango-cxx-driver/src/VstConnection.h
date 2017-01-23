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

#ifndef ARANGO_CXX_DRIVER_VST_CONNECTION_H
#define ARANGO_CXX_DRIVER_VST_CONNECTION_H 1

#include <atomic>
#include <mutex>
#include <map>
#include <deque>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <fuerte/connection_interface.h>
#include <fuerte/vst.h>

// naming in this file will be closer to asio for internal functions and types
// functions that are exposed to other classes follow ArangoDB conding conventions

namespace arangodb { namespace fuerte { inline namespace v1 {

class Loop;

namespace vst {

class VstConnection : public std::enable_shared_from_this<VstConnection>, public ConnectionInterface {

// Connection object that handles sending and receiving of Velocystream
// Messages.
//
// A VstConnection tries to create a connection to one of the given peers on a
// a socket. Then it tries to do an asyncronous ssl handshake. When the
// handshake is done it starts the async read loop on the socket. And tries a
// first write.
//
// The startRead function uses the function handleRead as handler for the
// async_read on the socket. This handler calls start Read as soon it has taken
// all relevant data from the socket. so The loop runs until the vstConnection
// is stopped.
//
// startWrite will be Triggered after establishing a connection or when new
// data has been queued for sending. Then startWrite and handleWrite call each
// other in the same way as startRead / handleRead but stop doing so as soon as
// the writeQueue is empty.
//

public:
  VstConnection(detail::ConnectionConfiguration);

public:
  // this function prepares the request for sending
  // by creating a RequestItem and setting:
  //  - a messageid
  //  - the buffer to be send
  // this item is then moved to the request queue
  // and a write action is triggerd when there is
  // no other write in progress
  MessageID sendRequest(std::unique_ptr<Request>
                       ,OnErrorCallback
                       ,OnSuccessCallback) override;

  // synchronous operation for sending Requests implemented using the
  // asynchronous operation and a condition variable
  std::unique_ptr<Response> sendRequest(std::unique_ptr<Request>) override;

private:
  // SOCKET HANDLING /////////////////////////////////////////////////////////
  void initSocket();
  void shutdownSocket();
  void shutdownConnection();
  void restartConnection();
  virtual void start() override { initSocket(); }

  //handler to be posted to loop
  //this handler call their handle counterpart on completion

  // establishes connection and initiates handshake
  void startConnect(boost::asio::ip::tcp::resolver::iterator);
  void handleConnect(boost::system::error_code const& ec, boost::asio::ip::tcp::resolver::iterator);

  // does handshake and starts async read and write
  void startHandshake();

  void finishInitialization();

  // reads data from socket with boost::asio::async_read
  void startRead();
  // handler for boost::asio::async_read that extracs chunks form the network
  // takes complete chunks form the socket and starts a new read action. After
  // triggering the next read it processes the received data.
  void handleRead(boost::system::error_code const&, std::size_t transferred);

  // writes data form task queue to network using boost::asio::async_write
  void startWrite(bool possiblyEmpty = false);
  // handler for boost::asio::async_wirte that calls startWrite as long as there is new data
  void handleWrite(boost::system::error_code const&, std::size_t transferred, std::shared_ptr<RequestItem>);

private:
  // TODO FIXME -- fix aligenment when done so mutexes are not on the same cacheline etc
  std::shared_ptr<Loop> _asioLoop;
  ::boost::asio::io_service* _ioService;
  detail::ConnectionConfiguration _configuration;
  ::boost::asio::ip::tcp::resolver::iterator _endpoints;
  ::std::atomic_int _handlercount;
  ::std::atomic_uint_least64_t _messageId;
  ::std::shared_ptr<::boost::asio::ip::tcp::socket> _socket;
  ::boost::asio::ssl::context _context;
  ::std::shared_ptr<::boost::asio::ssl::stream<::boost::asio::ip::tcp::socket&>> _sslSocket;
  ::std::atomic_bool _pleaseStop;
  ::std::atomic_bool _stopped;
  ::boost::asio::deadline_timer _deadline;
  ::boost::asio::ip::tcp::endpoint _peer;
  ::boost::asio::streambuf _receiveBuffer;
  ::std::mutex _sendQueueMutex;
  ::std::deque<std::shared_ptr<RequestItem>> _sendQueue;
  ::std::mutex _mapMutex;
  ::std::map<MessageID,std::shared_ptr<RequestItem>> _messageMap;
  ::std::atomic_bool _connected;
  ::std::mutex _connectedMutex;
};

}
}}}
#endif
