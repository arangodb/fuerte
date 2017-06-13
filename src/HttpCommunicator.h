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
/// @author Ewout Prangsma
////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef ARANGO_CXX_DRIVER_HTTP_COMMUNICATOR_H
#define ARANGO_CXX_DRIVER_HTTP_COMMUNICATOR_H 1

#include <fuerte/loop.h>
#include <fuerte/helper.h>
#include <fuerte/message.h>
#include <fuerte/types.h>
#include <fuerte/FuerteLogger.h>

#include <curl/curl.h>
#include <chrono>
#include <mutex>

#include <atomic>

#include "CurlMultiAsio.h"

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
  HttpCommunicator(const std::shared_ptr<asio_io_service>& io_service);
  ~HttpCommunicator();

 public:
  uint64_t queueRequest(Destination, std::unique_ptr<Request>, Callbacks);
  bool used() { return _useCount; }
  uint64_t addUser(){ return ++_useCount; }
  uint64_t delUser(){ return --_useCount; }
  int requestsLeft() { return _curlm->requestsLeft(); }

 private:
  // NewRequest holds all data needed to create a new request.
  struct NewRequest {
    Destination _destination;
    std::unique_ptr<Request> _fuRequest;
    Callbacks _callbacks;
    Options _options;
  };

  // RequestItem contains all data of a single request that is ongoing.
  class RequestItem {
   public:
    RequestItem(NewRequest request)
        : _request(std::move(request)),
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

    RequestItem(RequestItem const& other) = delete;
    RequestItem& operator=(RequestItem const& other) = delete;

    // return the CURL EASY handle.
    inline CURL* handle() const { return _handle; }

   public:
    NewRequest _request;
    std::string _requestBody;
    struct curl_slist* _requestHeaders;

    mapss _responseHeaders;
    std::chrono::steady_clock::time_point _startTime;
    std::string _responseBody;

    char _errorBuffer[CURL_ERROR_SIZE];
   private:
    CURL* _handle;
  };

    // MessageStore keeps a thread safe list of all requests that are "in-flight".
  class MessageStore {
   public:
    // add a given item to the store (indexed by its ID).
    void add(std::shared_ptr<RequestItem> item) {
      std::lock_guard<std::mutex> lockMap(_mutex);
      _map.emplace(item->_request._fuRequest->messageID, item);
    }

    // findByID returns the item with given ID or nullptr is no such ID is
    // found in the store.
    std::shared_ptr<RequestItem> findByID(MessageID id) {
      std::lock_guard<std::mutex> lockMap(_mutex);
      auto found = _map.find(id);
      if (found == _map.end()) {
        // ID not found
        return std::shared_ptr<RequestItem>();
      }
      return found->second;
    }

    // removeByID removes the item with given ID from the store.
    void removeByID(MessageID id) {
      std::lock_guard<std::mutex> lockMap(_mutex);
      _map.erase(id);
    }

    // Notify all items that their being cancelled (by calling their onError)
    // and remove all items from the store.
    void cancelAll() {
      std::lock_guard<std::mutex> lockMap(_mutex);
      for (auto& item : _map) {
        item.second->_request._callbacks._onError(errorToInt(ErrorCondition::CanceledDuringReset),
                              std::move(item.second->_request._fuRequest), nullptr);
      }
      _map.clear();
    }

    // size returns the number of elements in the store.
    size_t size() {
      std::lock_guard<std::mutex> lockMap(_mutex);
      return _map.size();
    }

    // empty returns true when there are no elements in the store, false
    // otherwise.
    bool empty(bool unlocked = false) {
      if (unlocked) {
        return _map.empty();
      } else {
        std::lock_guard<std::mutex> lockMap(_mutex);
        return _map.empty();
      }
    }

    // mutex provides low level access to the mutex, used for shared locking.
    std::mutex& mutex() { return _mutex; }

    // keys returns a string representation of all MessageID's in the store.
    std::string keys() {
      std::lock_guard<std::mutex> lockMap(_mutex);
      return mapToKeys(_map);
    }

   private:
    std::mutex _mutex;
    std::map<MessageID, std::shared_ptr<RequestItem>> _map;
  };
  MessageStore _messageStore;

 private:
  static size_t readBody(void*, size_t, size_t, void*);
  static size_t readHeaders(char* buffer, size_t size, size_t nitems,
                            void* userdata);
  static int curlDebug(CURL*, curl_infotype, char*, size_t, void*);
  static void logHttpHeaders(std::string const&, std::string const&);
  static void logHttpBody(std::string const&, std::string const&);

 private:
  void createRequestIem(NewRequest);
  void handleResult(CURL*, CURLcode);
  void transformResult(CURL*, mapss&&, std::string const&, Response*);

  /// @brief curl will strip standalone ".". ArangoDB allows using . as a key
  /// so this thing will analyse the url and urlencode any unsafe .'s
  std::string createSafeDottedCurlUrl(std::string const& originalUrl);

 private:
  //static std::mutex _curlLock;
  std::shared_ptr<CurlMultiAsio> _curlm;

  std::atomic<uint64_t> _useCount;
  int _stillRunning;
};

}
}
}
}

#endif
