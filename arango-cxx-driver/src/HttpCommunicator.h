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
////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef ARANGO_CXX_DRIVER_HTTP_COMMUNICATOR_H
#define ARANGO_CXX_DRIVER_HTTP_COMMUNICATOR_H 1

#include <fuerte/message.h>
#include <fuerte/types.h>
#include <fuerte/FuerteLogger.h>

#include <curl/curl.h>
#include <chrono>
#include <mutex>

#include <atomic>

namespace arangodb {
namespace fuerte {
inline namespace v1 {
namespace http {
typedef std::string Destination;
// -----------------------------------------------------------------------------
// --SECTION--                                                   class Callbacks
// -----------------------------------------------------------------------------

class Callbacks {
 public:
  Callbacks() {}

  Callbacks(OnSuccessCallback onSuccess, OnErrorCallback onError)
      : _onSuccess(onSuccess), _onError(onError) {}

 public:
  OnSuccessCallback _onSuccess;
  OnErrorCallback _onError;
};


// -----------------------------------------------------------------------------
// --SECTION--                                                     class Options
// -----------------------------------------------------------------------------

class Options {
 public:
  double requestTimeout = 120.0;
  double connectionTimeout = 2.0;
};

// -----------------------------------------------------------------------------
// --SECTION--                                            class HttpCommunicator
// -----------------------------------------------------------------------------

class HttpCommunicator {
 public:
  HttpCommunicator();
  ~HttpCommunicator();

 public:
  uint64_t queueRequest(Destination, std::unique_ptr<Request>, Callbacks);
  int workOnce();
  void wait();
  bool used(){ return _useCount; }
  uint64_t addUser(){ return ++_useCount; }
  uint64_t delUser(){ return --_useCount; }
  int requestsLeft(){ return _stillRunning; }

 private:
  struct NewRequest {
    Destination _destination;
    std::unique_ptr<Request> _fuRequest;
    Callbacks _callbacks;
    Options _options;
  };

  class RequestInProgress {
   public:
    RequestInProgress(NewRequest request)
        : _request(std::move(request)),
          _requestHeaders(nullptr),
          _startTime(std::chrono::steady_clock::now()) {
      _errorBuffer[0] = '\0';
    }

    ~RequestInProgress() {
      if (_requestHeaders != nullptr) {
        curl_slist_free_all(_requestHeaders);
      }
    }

    RequestInProgress(RequestInProgress const& other) = delete;
    RequestInProgress& operator=(RequestInProgress const& other) = delete;

   public:
    NewRequest _request;
    std::string _requestBody;
    struct curl_slist* _requestHeaders;

    mapss _responseHeaders;
    std::chrono::steady_clock::time_point _startTime;
    std::string _responseBody;

    char _errorBuffer[CURL_ERROR_SIZE];
  };

  struct CurlHandle {
    explicit CurlHandle(RequestInProgress* rip) : _handle(nullptr), _rip(rip) {
      _handle = curl_easy_init();

      if (_handle == nullptr) {
        throw std::bad_alloc();
      }

      curl_easy_setopt(_handle, CURLOPT_PRIVATE, _rip.get());
#ifdef CURLOPT_PATH_AS_IS
      curl_easy_setopt(_handle, CURLOPT_PATH_AS_IS, 1L);
#endif
    }

    ~CurlHandle() {
      if (_handle != nullptr) {
        curl_easy_cleanup(_handle);
      }
    }

    CurlHandle(CurlHandle& other) = delete;
    CurlHandle& operator=(CurlHandle& other) = delete;

    CURL* _handle;
    std::unique_ptr<RequestInProgress> _rip;
  };

 private:
  static size_t readBody(void*, size_t, size_t, void*);
  static size_t readHeaders(char* buffer, size_t size, size_t nitems,
                            void* userdata);
  static int curlDebug(CURL*, curl_infotype, char*, size_t, void*);
  static void logHttpHeaders(std::string const&, std::string const&);
  static void logHttpBody(std::string const&, std::string const&);

 private:
  void createRequestInProgress(NewRequest);
  void handleResult(CURL*, CURLcode);
  void transformResult(CURL*, mapss&&, std::string const&, Response*);

  /// @brief curl will strip standalone ".". ArangoDB allows using . as a key
  /// so this thing will analyse the url and urlencode any unsafe .'s
  std::string createSafeDottedCurlUrl(std::string const& originalUrl);

 private:
  std::mutex _newRequestsLock;
  static std::mutex _curlLock;
  std::vector<NewRequest> _newRequests;

  std::unordered_map<uint64_t, std::unique_ptr<CurlHandle>> _handlesInProgress;
  CURLM* _curl;
  CURLMcode _mc;
  curl_waitfd _wakeup;
  std::atomic<uint64_t> _useCount;
  int _stillRunning;

#ifdef _WIN32
  SOCKET _socks[2];
#else
  int _fds[2];
#endif
};
}
}
}
}

#endif
