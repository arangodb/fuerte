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
/// @author Ewout Prangsma
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef ARANGO_CXX_DRIVER_SERVER
#define ARANGO_CXX_DRIVER_SERVER

#include <utility>
#include <memory>
#include <iostream>
#include <thread>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

// run / runWithWork / poll for Loop mapping to ioservice
// free function run with threads / with thread group barrier and work

namespace arangodb { namespace fuerte { inline namespace v1 {

namespace vst {
  class VstConnection;
}

namespace http {
  class HttpConnection;
}

namespace impl {
  class VpackInit;
}

// need partial rewrite so it can be better integrated in client applications

typedef boost::asio::io_context asio_io_context;
typedef boost::asio::executor_work_guard<boost::asio::io_context::executor_type> asio_work_guard;

// GlobalService is intended to be instantiated once for the entire 
// lifetime of a program using fuerte. 
// It initializes all global state, needed by fuerte.
class GlobalService {
 public:
  // get GlobalService singelton.
  static GlobalService& get() {
    static GlobalService service;
    return service;
  }

 private:
  GlobalService();
  ~GlobalService();

  // Prevent copying
  GlobalService(GlobalService const& other) = delete;
  GlobalService& operator=(GlobalService const& other) = delete;

 private:
  std::unique_ptr<impl::VpackInit> _vpack_init; 
};

// EventLoopService implements multi-threaded event loops for
// boost io_service as well as curl HTTP.
class EventLoopService {
  //friend class vst::VstConnection;
  friend class http::HttpConnection;
  friend class ConnectionBuilder;

 public:
  // Initialize an EventLoopService with a given number of threads and a new io_service.
  EventLoopService(unsigned int threadCount = 1);
  // Initialize an EventLoopService with a given number of threads and a given io_service.
  // Initialize an EventLoopService with a given number of threads and a given io_service.
  /*EventLoopService(const std::shared_ptr<::boost::asio::io_context>& io_service)
      : global_service_(GlobalService::get()),
        io_service_(io_service), 
        working_(new asio_work(*io_service)) {
    while (threadCount > 0) {
      auto worker = std::bind(&EventLoopService::run, this);
      threadGroup_.add_thread(new boost::thread(worker));
      threadCount--;
    }
  }*/
  virtual ~EventLoopService();

  // Prevent copying
  EventLoopService(EventLoopService const& other) = delete;
  EventLoopService& operator=(EventLoopService const& other) = delete;

 protected:

  // handleRunException is called when an exception is thrown in run.
  virtual void handleRunException(std::exception const& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  // io_service returns a reference to the boost io_service.
  std::shared_ptr<asio_io_context>& io_context() { 
    return _ioContexts[0]; 
  }

    // io_service returns a reference to the boost io_service.
  std::shared_ptr<asio_io_context>& nextIOContext() { 
    return _ioContexts[_lastUsed.fetch_add(1) % _ioContexts.size()];
  }

 private:
  GlobalService& global_service_;
  std::atomic<uint32_t> _lastUsed;

  /// io contexts
  std::vector<std::shared_ptr<asio_io_context>> _ioContexts;
  /// Used to keep the io-context alive.
  std::vector<asio_work_guard> _guards;
  /// Threads powering each io_context
  std::vector<std::thread> _threads;
};

}}}
#endif

