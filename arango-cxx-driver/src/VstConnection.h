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

#include <functional>
#include <atomic>
#include <mutex>
#include <fuerte/connection_interface.h>
#include "asio/asio.h"
#include "asio/Socket.h"
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ssl.hpp>
#include <map>
#include <deque>
#include <fuerte/vst.h>


// naming in this file will be closer to asio for internal functions and types
// functions that are exposed to other classes follow ArangoDB conding conventions

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {

class VstConnection : public std::enable_shared_from_this<VstConnection>, public ConnectionInterface {
public:
  VstConnection(detail::ConnectionConfiguration);

public:
  void start() override {}

  MessageID sendRequest(std::unique_ptr<Request>
                  ,OnErrorCallback
                  ,OnSuccessCallback) override;

  std::unique_ptr<Response> sendRequest(std::unique_ptr<Request>) override;

private:
  // SOCKET HANDLING /////////////////////////////////////////////////////////
  void initSocket(boost::asio::ip::tcp::resolver::iterator);
  void shutdownSocket();
  void shutdownConnection();
  void restartConnection();

  //handler to be posted to loop
  //this handler call their handle counterpart on completeion

  // establishes connection and initiates handshake
  void startConnect(boost::asio::ip::tcp::resolver::iterator);
  void handleConnect(boost::system::error_code const& ec, boost::asio::ip::tcp::resolver::iterator);

  // does handshake and starts async read and write
  void startHandshake();
  void handleHandshake();

  // reads data from socket and filles task queue
  void startRead();
  void handleRead(boost::system::error_code const&, std::size_t transferred);

  // writes data form task queue to network
  void startWrite();
  void handleWrite(boost::system::error_code const&, std::size_t transferred, std::shared_ptr<RequestItem>);

  // TASK HANDLING /////////////////////////////////////////////////////////

  // TO BE CREATED

private:
  std::shared_ptr<asio::Loop> _asioLoop;
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
  //boost::posix_time::milliseconds _keepAliveTimeout;
  //boost::asio::deadline_timer _keepAliveTimer;
  //bool const _useKeepAliveTimer;
  //bool _keepAliveTimerActive;

};

}}}}
#endif
