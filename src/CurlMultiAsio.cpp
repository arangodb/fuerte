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
/// @author Andreas Streichardt
/// @author Frank Celler
/// @author Jan Christoph Uhde
////////////////////////////////////////////////////////////////////////////////

#include "CurlMultiAsio.h"
#include <fcntl.h>
#include <cassert>
#include <sstream>
#include <atomic>
#include <cassert>

#include <boost/bind.hpp>
#include <fuerte/helper.h>
#include <fuerte/FuerteLogger.h>

namespace arangodb {
namespace fuerte {
inline namespace v1 {
namespace http {

// Convert a CURL action to a string
inline const char* curlWhat(int what) {
  switch (what) {
    case CURL_POLL_IN:
      return "IN";
    case CURL_POLL_OUT:
      return "OUT";
    case CURL_POLL_INOUT:
      return "INOUT";
    case CURL_POLL_REMOVE:
      return "REMOVE";
    default:
      return "?";
  }
}
 

// Initialize out CURL MULTI and prepare callbacks.
CurlMultiAsio::CurlMultiAsio(boost::asio::io_service& io_service, CurlMultiAsio::RequestDoneCallback request_done_cb) :
  _io_service(io_service),
  _request_done_cb(request_done_cb),
  _timer(io_service),
  _requests_left(0) {

  // Create CURLMULTI
  _multi = curl_multi_init();
  if (!_multi) {
    throw std::logic_error("curl_multi_init failed");
  }

  // Initialize callbacks
  curl_multi_setopt(_multi, CURLMOPT_SOCKETFUNCTION, bind_socket_cb);
  curl_multi_setopt(_multi, CURLMOPT_SOCKETDATA, this);
  curl_multi_setopt(_multi, CURLMOPT_TIMERFUNCTION, bind_multi_timer_cb);
  curl_multi_setopt(_multi, CURLMOPT_TIMERDATA, this);
}

// Cleanup
CurlMultiAsio::~CurlMultiAsio() {
  if (_multi) {
    curl_multi_cleanup(_multi);
  }
}

// addRequest connect a prepare CURL EASY request to our CURL MULTI instance.
// It configures callbacks needed to connect the sockets.
void CurlMultiAsio::addRequest(CURL *easyHandle) {
  // Initialize callback
  curl_easy_setopt(easyHandle, CURLOPT_OPENSOCKETFUNCTION, bind_open_socket);
  curl_easy_setopt(easyHandle, CURLOPT_OPENSOCKETDATA, this);
  curl_easy_setopt(easyHandle, CURLOPT_CLOSESOCKETFUNCTION, bind_close_socket);
  curl_easy_setopt(easyHandle, CURLOPT_CLOSESOCKETDATA, this);
 
  FUERTE_LOG_HTTPTRACE << "Adding easy " << easyHandle << " to our multi" << std::endl;
  {
    std::unique_lock<std::recursive_mutex> lock(_multi_mutex);
    auto rc = curl_multi_add_handle(_multi, easyHandle);
    assertCurlOK("addRequest", rc);
  }
}

// Timer callback (CURLMOPT_TIMERFUNCTION)
int CurlMultiAsio::multi_timer_cb(CURLM *multi, long timeout_ms, void *userp) 
{
  FUERTE_LOG_HTTPTRACE << "multi_timer_cb: timeout_ms " << timeout_ms << std::endl;

  // cancel running timer
  _timer.cancel();
 
  if (timeout_ms > 0) {
    // update timer
    _timer.expires_from_now(boost::posix_time::millisec(timeout_ms));
    _timer.async_wait(boost::bind(&CurlMultiAsio::timer_cb, this, boost::asio::placeholders::error));
  }
  else if (timeout_ms == 0) {
    // call timeout function as soon as possible 
    auto self = shared_from_this();
    _io_service.post([this, self]{
      boost::system::error_code error; /*success*/
      timer_cb(error);
    }); 
  }

  FUERTE_LOG_HTTPTRACE << "multi_timer_cb: timeout_ms " << timeout_ms << " done" << std::endl;

  return 0;
}

// Called by asio when our timeout expires
void CurlMultiAsio::timer_cb(const boost::system::error_code & error)
{
  if(!error) {
    FUERTE_LOG_HTTPTRACE << "timer_cb: " << std::endl;
 
    int still_running;
    {
      std::unique_lock<std::recursive_mutex> lock(_multi_mutex);
      FUERTE_LOG_HTTPTRACE << "timer_cb: before curl_multi_socket_action" << std::endl;
      auto rc = curl_multi_socket_action(_multi, CURL_SOCKET_TIMEOUT, 0, &still_running);
      FUERTE_LOG_HTTPTRACE << "timer_cb: after curl_multi_socket_action" << std::endl;
      assertCurlOK("timer_cb: curl_multi_socket_action", rc);
    }
    check_multi_info(still_running);
  }
}
 
// Throw an exception if the given code is not equal to CURLM_OK.
void CurlMultiAsio::assertCurlOK(const char *where, CURLMcode code)
{
  if(CURLM_OK != code) {
    const char *s;
    switch(code) {
    case CURLM_CALL_MULTI_PERFORM:
      s = "CURLM_CALL_MULTI_PERFORM";
      break;
    case CURLM_BAD_HANDLE:
      s = "CURLM_BAD_HANDLE";
      break;
    case CURLM_BAD_EASY_HANDLE:
      s = "CURLM_BAD_EASY_HANDLE";
      break;
    case CURLM_OUT_OF_MEMORY:
      s = "CURLM_OUT_OF_MEMORY";
      break;
    case CURLM_INTERNAL_ERROR:
      s = "CURLM_INTERNAL_ERROR";
      break;
    case CURLM_UNKNOWN_OPTION:
      s = "CURLM_UNKNOWN_OPTION";
      break;
    case CURLM_LAST:
      s = "CURLM_LAST";
      break;
    default:
      s = "CURLM_unknown";
      break;
    case CURLM_BAD_SOCKET:
      s = "CURLM_BAD_SOCKET";
      FUERTE_LOG_ERROR << "ERROR: " << where << " returns " << s << std::endl;
      /* ignore this error */ 
      return;
    }
 
    FUERTE_LOG_ERROR << "ERROR: " << where << " returns " << s << std::endl;
 
    exit(code);
  }
}
 
// Check for completed transfers, and perform callback on them.
void CurlMultiAsio::check_multi_info(int still_running)
{
  CURLMsg *msg;
  int msgs_left;
 
  _requests_left = still_running;
  FUERTE_LOG_HTTPTRACE << "check_multi_info: still_running=" << still_running << std::endl;
 
  while ((msg = curl_multi_info_read(_multi, &msgs_left))) {
    if (msg->msg == CURLMSG_DONE) {
      auto easy = msg->easy_handle;
      auto result = msg->data.result;

      char *eff_url;
      curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &eff_url);
      FUERTE_LOG_HTTPTRACE << "curl message done: " << eff_url << " => result=" << result << std::endl;

      // Remove handle from multi
      {
        std::unique_lock<std::recursive_mutex> lock(_multi_mutex);
        auto rc = curl_multi_remove_handle(_multi, easy);
        assertCurlOK("check_multi_info: curl_multi_remove_handle", rc);
      }

      // Invoke callback in the event loop
      auto self = shared_from_this();
      _io_service.post([this, self, easy, result]{ _request_done_cb(easy, result); });
    }
  }
}

// Called by asio when there is an action on a socket
void CurlMultiAsio::event_cb(curl_socket_t s, int action,
                             const boost::system::error_code& error, CurlMultiAsio::SocketInfo* socketp) {
  FUERTE_LOG_HTTPTRACE << "event_cb: action=" << curlWhat(action) << std::endl;

  {
    std::unique_lock<std::mutex> lock(_map_mutex);
    if (_socket_map.find(s) == _socket_map.end()) {
      FUERTE_LOG_HTTPTRACE << "event_cb: socket already closed" << std::endl;
      return;
    }
  }

  // make sure the event matches what are wanted
  if (socketp->action == action || socketp->action == CURL_POLL_INOUT) {
    if (error) {
      action = CURL_CSELECT_ERR;
    }
    int still_running;
    {
      std::unique_lock<std::recursive_mutex> lock(_multi_mutex);
      auto rc = curl_multi_socket_action(_multi, s, action, &still_running);
      assertCurlOK("event_cb: curl_multi_socket_action", rc);
    }
    check_multi_info(still_running);

    if (still_running <= 0) {
      FUERTE_LOG_HTTPTRACE << "last transfer done, kill timeout" << std::endl;
      _timer.cancel();
    }

    // keep on watching.
    // the socket may have been closed and/or action may have been changed
    // in curl_multi_socket_action(), so check them both
    if (!error && (socketp->action == action || socketp->action == CURL_POLL_INOUT)) {
      boost::asio::ip::tcp::socket* tcp_socket;
      {
        std::unique_lock<std::mutex> lock(_map_mutex);
        auto it = _socket_map.find(s);
        if (it == _socket_map.end()) {
          // Socket no longer found
          return;
        }
        tcp_socket = it->second;
      }
      auto self = shared_from_this();
      if (action == CURL_POLL_IN) {
        tcp_socket->async_read_some(
            boost::asio::null_buffers(),
            boost::bind(&CurlMultiAsio::event_cb, self, s, action, _1, socketp));
      }
      if (action == CURL_POLL_OUT) {
        tcp_socket->async_write_some(
            boost::asio::null_buffers(),
            boost::bind(&CurlMultiAsio::event_cb, self, s, action, _1, socketp));
      }
    }
  }
}

void CurlMultiAsio::set_socket(CurlMultiAsio::SocketInfo *socketp, curl_socket_t s, CURL *easy, int action, int oldAction)
{
  FUERTE_LOG_HTTPTRACE << "set_socket: socket=" << s << ", action=" << curlWhat(action) << " socketp=" << socketp << std::endl;

  // Lookup tcp_socket for the given curl socket.
  boost::asio::ip::tcp::socket * tcp_socket = nullptr;
  {
    std::unique_lock<std::mutex> lock(_map_mutex);
    auto it = _socket_map.find(s);
    if(it == _socket_map.end()) {
      FUERTE_LOG_HTTPTRACE << "socket " << s << " is a c-ares socket, ignoring" << std::endl;
      return;
    }
    tcp_socket = it->second;
  }
 
  // Store action for later
  socketp->action = action;
 
  auto self = shared_from_this();
  if (action == CURL_POLL_IN) {
    FUERTE_LOG_HTTPTRACE << "watching for socket to become readable" << std::endl;
    if (oldAction != CURL_POLL_IN && oldAction != CURL_POLL_INOUT) {
      tcp_socket->async_read_some(boost::asio::null_buffers(),
                                  boost::bind(&CurlMultiAsio::event_cb, self, s, CURL_POLL_IN, _1, socketp));
    }
  }
  else if (action == CURL_POLL_OUT) {
    FUERTE_LOG_HTTPTRACE << "watching for socket to become writable" << std::endl;
    if (oldAction != CURL_POLL_OUT && oldAction != CURL_POLL_INOUT) {
      tcp_socket->async_write_some(boost::asio::null_buffers(),
                                   boost::bind(&CurlMultiAsio::event_cb, self, s, CURL_POLL_OUT, _1, socketp));
    }
  }
  else if (action == CURL_POLL_INOUT) {
    FUERTE_LOG_HTTPTRACE << "watching for socket to become readable & writable" << std::endl;
    if (oldAction != CURL_POLL_IN && oldAction != CURL_POLL_INOUT) {
      tcp_socket->async_read_some(boost::asio::null_buffers(),
                                  boost::bind(&CurlMultiAsio::event_cb, self, s, CURL_POLL_IN, _1, socketp));
    }
    if (oldAction != CURL_POLL_OUT && oldAction != CURL_POLL_INOUT) {
      tcp_socket->async_write_some(boost::asio::null_buffers(),
                                   boost::bind(&CurlMultiAsio::event_cb, self, s, CURL_POLL_OUT, _1, socketp));
    }
  }
}
 
// Allocate SocketInfo data.
void CurlMultiAsio::add_socket(curl_socket_t s, CURL *easy, int action)
{
  /* fdp is used to store current action */ 
  auto socketp = new CurlMultiAsio::SocketInfo{.action = 0};
  {
    std::unique_lock<std::recursive_mutex> lock(_multi_mutex);
    curl_multi_assign(_multi, s, socketp);
  }

  // Continue
  set_socket(socketp, s, easy, action, 0);
}
 
// Clean up any SocketInfo data
void CurlMultiAsio::remove_socket(CurlMultiAsio::SocketInfo *socketp, curl_socket_t s, CURL *easy)
{
  FUERTE_LOG_HTTPTRACE << "remove_socket: " << std::endl;
  {
    std::unique_lock<std::recursive_mutex> lock(_multi_mutex);
    curl_multi_assign(_multi, s, nullptr);
  }
  if (socketp) {
    delete socketp;
  }
}
 
/* CURLMOPT_SOCKETFUNCTION */ 
int CurlMultiAsio::socket_cb(CURL *easy,      /* easy handle */
                    curl_socket_t s, /* socket */
                    int what,        /* describes the socket */
                    void *userp,     /* private callback pointer */
                    CurlMultiAsio::SocketInfo *socketp)  /* private socket pointer */
{
  FUERTE_LOG_HTTPTRACE << "socket_cb: socket=" << s << ", what=" << curlWhat(what) << std::endl;
 
  if (what == CURL_POLL_REMOVE) {
    remove_socket(socketp, s, easy);
  } else {
    if (!socketp) {
      FUERTE_LOG_HTTPTRACE << "Adding data: " << curlWhat(what) << std::endl;
      add_socket(s, easy, what);
    } else {
      FUERTE_LOG_HTTPTRACE << "Changing action from " << curlWhat(socketp->action) << " to " << curlWhat(what) << std::endl;
      set_socket(socketp, s, easy, what, socketp->action);
    }
  }
 
  return 0;
}
 
// Open a socket (CURLOPT_OPENSOCKETFUNCTION callback  function)
curl_socket_t CurlMultiAsio::open_socket(curlsocktype purpose, struct curl_sockaddr *address)
{
  FUERTE_LOG_HTTPTRACE << "open_socket" << std::endl;
 
  curl_socket_t sockfd = CURL_SOCKET_BAD;
 
  // restrict to IPv4 
  if (purpose == CURLSOCKTYPE_IPCXN && address->family == AF_INET) {
    /* create a tcp socket object */
    auto tcp_socket = new boost::asio::ip::tcp::socket(_io_service);

    /* open it and get the native handle*/ 
    boost::system::error_code ec;
    tcp_socket->open(boost::asio::ip::tcp::v4(), ec);
 
    if (ec) {
      // An error occurred 
      FUERTE_LOG_ERROR << "Couldn't open socket [" << ec << "][" << ec.message() << "]" << std::endl;
      FUERTE_LOG_HTTPTRACE << "ERROR: Returning CURL_SOCKET_BAD to signal error" << std::endl;
    }
    else {
      sockfd = tcp_socket->native_handle();
      FUERTE_LOG_HTTPTRACE << "Opened socket " << sockfd << std::endl;
 
      // save the socket mapping
      {
        std::unique_lock<std::mutex> lock(_map_mutex);
        _socket_map.insert(std::pair<curl_socket_t, boost::asio::ip::tcp::socket *>(sockfd, tcp_socket));
      }
    }
  }

  FUERTE_LOG_HTTPTRACE << "open_socket done; returning socket " << sockfd << std::endl;
 
  return sockfd;
}
 
// Close a socket (CURLOPT_CLOSESOCKETFUNCTION callback function)
int CurlMultiAsio::close_socket(curl_socket_t item)
{
  FUERTE_LOG_HTTPTRACE << "close_socket: " << item << std::endl;
 
  {
    std::unique_lock<std::mutex> lock(_map_mutex);
    auto it = _socket_map.find(item); 
    if (it != _socket_map.end()) {
      delete it->second;
      _socket_map.erase(it);
    }
  }
 
  return 0;
}

}
}
}
}
