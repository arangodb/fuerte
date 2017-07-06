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
/// @author Ewout Prangsma
/// @author Frank Celler
////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef ARANGO_CXX_DRIVER_CURL_MULTI_ASIO_H
#define ARANGO_CXX_DRIVER_CURL_MULTI_ASIO_H 1

#include <fuerte/types.h>
#include <fuerte/FuerteLogger.h>

#include <boost/asio.hpp>
#include <curl/curl.h>
#include <chrono>
#include <mutex>

#include <atomic>

namespace arangodb {
namespace fuerte {
inline namespace v1 {
namespace http {

// CurlMultiAsio makes CURLMULTI play nice in a boost::asio environment.
class CurlMultiAsio : public std::enable_shared_from_this<CurlMultiAsio> {
 public:
  using RequestDoneCallback = std::function<void(CURL* easyHandle, CURLcode result)>;

  CurlMultiAsio(boost::asio::io_service& io_service, RequestDoneCallback request_done_cb);
  ~CurlMultiAsio();

  // Prevent copying
  CurlMultiAsio(CurlMultiAsio const& other) = delete;
  CurlMultiAsio& operator=(CurlMultiAsio const& other) = delete;

  // addRequest connect a prepare CURL EASY request to our CURL MULTI instance.
  // It configures callbacks needed to connect the sockets.
  void addRequest(CURL *easyHandle);

  // Return the number of unfinished requests.
  int requestsLeft() {
    return _requests_left;
  }

 private:
  typedef struct {
    int action;
  } SocketInfo;

  // Check for completed transfers, and remove their easy handles 
  void check_multi_info(int still_running);
  // Timer callback (CURLMOPT_TIMERFUNCTION)
  int multi_timer_cb(CURLM *multi, long timeout_ms, void *userp);
  // Called by asio when our timeout expires
  void timer_cb(const boost::system::error_code & error);
  // Open a socket (CURLOPT_OPENSOCKETFUNCTION callback function)
  curl_socket_t open_socket(curlsocktype purpose, struct curl_sockaddr *address);
  // Close a socket (CURLOPT_CLOSESOCKETFUNCTION callback function)
  int close_socket(curl_socket_t item);
  // Called by asio when there is an action on a socket 
  void event_cb(curl_socket_t s, int action, const boost::system::error_code & error, SocketInfo *socketp);
  // CURLMOPT_SOCKETFUNCTION callback
  int socket_cb(CURL* easy, curl_socket_t s, int what, void* userp, SocketInfo* socketp);
  void set_socket(SocketInfo *socketp, curl_socket_t s, CURL *easy, int action, int oldAction);
  // Allocate SocketInfo for given socket.
  void add_socket(curl_socket_t s, CURL *easy, int action);
  // Clean up any SocketInfo data
  void remove_socket(SocketInfo *socketp, curl_socket_t s, CURL *easy);

  // Throw an exception if the given code is not equal to CURLM_OK.
  void assertCurlOK(const char *where, CURLMcode code);

 private:
  // These bind methods are used as callback entrypoint for libcurl.
  // All the userp data must point to an instance of CurlMultiAsio.
  static int bind_multi_timer_cb(CURLM *multi, long timeout_ms, void *userp) {
    return static_cast<CurlMultiAsio*>(userp)->multi_timer_cb(multi, timeout_ms, userp);
  }
  static int bind_socket_cb(CURL *easy, curl_socket_t s, int what, void *userp, SocketInfo *socketp) {
    return static_cast<CurlMultiAsio*>(userp)->socket_cb(easy, s, what, userp, socketp);
  }
  static curl_socket_t bind_open_socket(void *userp, curlsocktype purpose, struct curl_sockaddr *address) {
    return static_cast<CurlMultiAsio*>(userp)->open_socket(purpose, address);
  }
  static int bind_close_socket(void *userp, curl_socket_t item) {
    return static_cast<CurlMultiAsio*>(userp)->close_socket(item);
  }


 private:
  boost::asio::io_service& _io_service;
  RequestDoneCallback _request_done_cb;
  std::recursive_mutex _multi_mutex;
  CURLM* _multi;
  std::mutex _timer_mutex;
  boost::asio::deadline_timer _timer;
  std::mutex _map_mutex;
  std::map<curl_socket_t, boost::asio::ip::tcp::socket*> _socket_map;
  int _requests_left;

 private:
  inline void recordNewAsyncCall(const char *where) {
    #if ENABLE_FUERTE_LOG_HTTPTRACE > 0
      auto ctr = ++_pendingAsyncCalls;
      FUERTE_LOG_HTTPTRACE << "Starting async call " << ctr << " in " << where << std::endl;
    #endif
  }
#if ENABLE_FUERTE_LOG_HTTPTRACE > 0
  std::atomic<uint64_t> _pendingAsyncCalls;
#endif
};

}
}
}
}

#endif
