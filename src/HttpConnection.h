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
/// @author John Bufton
/// @author Ewout Prangsma
////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef ARANGO_CXX_DRIVER_HTTP_CONNECTION_H
#define ARANGO_CXX_DRIVER_HTTP_CONNECTION_H 1

#include <chrono>
#include <mutex>
#include <atomic>
#include <stdexcept>

#include <fuerte/connection.h>
#include <fuerte/loop.h>
#include <fuerte/helper.h>
#include <fuerte/message.h>
#include <fuerte/types.h>
#include <fuerte/FuerteLogger.h>

#include <curl/curl.h>

#include "CallOnceRequestCallback.h"
#include "CurlMultiAsio.h"
#include "MessageStore.h"

namespace arangodb {
namespace fuerte {
inline namespace v1 {
namespace http {

typedef std::string Destination;

// HttpConnection implements a client->server connection using the HTTP protocol.
class HttpConnection : public Connection {
 public:
  explicit HttpConnection(EventLoopService&, detail::ConnectionConfiguration const&);
  virtual ~HttpConnection();

 public:
  // Start an asynchronous request.
  MessageID sendRequest(std::unique_ptr<Request>, RequestCallback) override;

  // Return the number of unfinished requests.
  std::size_t requestsLeft() override {
    return _curlm->requestsLeft();
  }

 private:
  class Options {
  public:
    double connectionTimeout = 2.0;
  };

  uint64_t queueRequest(Destination, std::unique_ptr<Request>, RequestCallback);

 private:
  // RequestItem contains all data of a single request that is ongoing.
  class RequestItem {
   public:
    RequestItem(const Destination& destination, std::unique_ptr<Request> request, RequestCallback callback)
        : _destination(destination),
          _request(std::move(request)),
          _callback(callback),
          _requestHeaders(nullptr),
          _startTime(std::chrono::steady_clock::now()) {
      _errorBuffer[0] = '\0';

      _handle = curl_easy_init();
      if (_handle == nullptr) {
        throw std::bad_alloc();
      }

      curl_easy_setopt(_handle, CURLOPT_PRIVATE, this);
#ifdef CURLOPT_PATH_AS_IS
      curl_easy_setopt(_handle, CURLOPT_PATH_AS_IS, 1L);
#endif
    }

    ~RequestItem() {
      if (_requestHeaders != nullptr) {
        curl_slist_free_all(_requestHeaders);
      }
      if (_handle != nullptr) {
        curl_easy_cleanup(_handle);
      }
    }

    // Prevent copying
    RequestItem(RequestItem const& other) = delete;
    RequestItem& operator=(RequestItem const& other) = delete;

    // return the CURL EASY handle.
    inline CURL* handle() const { return _handle; }

    inline MessageID messageID() { return _request->messageID; }
    inline void invokeOnError(Error e, std::unique_ptr<Request> req, std::unique_ptr<Response> res) { 
      _callback.invoke(e, std::move(req), std::move(res));
    }

   public:
    Destination _destination;
    std::unique_ptr<Request> _request;
    impl::CallOnceRequestCallback _callback;
    Options _options;
    std::string _requestBody;
    struct curl_slist* _requestHeaders;

    StringMap _responseHeaders;
    std::chrono::steady_clock::time_point _startTime;
    std::string _responseBody;

    char _errorBuffer[CURL_ERROR_SIZE];
   private:
    CURL* _handle;
  };

  MessageStore<RequestItem> _messageStore;

 private:
  static size_t readBody(void*, size_t, size_t, void*);
  static size_t readHeaders(char* buffer, size_t size, size_t nitems,
                            void* userdata);
  static int curlDebug(CURL*, curl_infotype, char*, size_t, void*);
  static void logHttpHeaders(std::string const&, std::string const&);
  static void logHttpBody(std::string const&, std::string const&);

 private:
  void createRequestItem(const Destination& destination, std::unique_ptr<Request> request, RequestCallback callback);
  void handleResult(CURL*, CURLcode);
  void transformResult(CURL*, StringMap&&, std::string const&, Response*);

  /// @brief curl will strip standalone ".". ArangoDB allows using . as a key
  /// so this thing will analyse the url and urlencode any unsafe .'s
  std::string createSafeDottedCurlUrl(std::string const& originalUrl);

 private:
  std::shared_ptr<CurlMultiAsio> _curlm;
  //int _stillRunning;
};

}
}
}
}

#endif
