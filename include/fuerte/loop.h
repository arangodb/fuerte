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

#include <boost/asio.hpp>
#include <boost/thread.hpp>

// run / runWithWork / poll for Loop mapping to ioservice
// free function run with threads / with thread group barrier and work

namespace arangodb { namespace fuerte { inline namespace v1 {

//class Work;
class Loop;

namespace vst {
  class VstConnection;
}

namespace http{
  class HttpCommunicator;
  class HttpConnection;
}

// need partial rewrite so it can be better integrated in client applications

typedef ::boost::asio::io_service asio_io_service;
typedef ::boost::asio::io_service::work asio_work;

// EventLoopService implements multi-threaded event loops for
// boost io_service as well as curl HTTP.
class EventLoopService {
  friend class vst::VstConnection;
  friend class http::HttpConnection;

 public:
  // Initialize an EventLoopService with a given number of threads and a new io_service.
  EventLoopService(unsigned int threadCount = 1);
  // Initialize an EventLoopService with a given number of threads and a given io_service.
  EventLoopService(unsigned int threadCount,
                   const std::shared_ptr<asio_io_service>& io_service,
                   const std::shared_ptr<http::HttpCommunicator>& httpCommunicator)
      : io_service_(io_service), 
      working_(new asio_work(*io_service)),
      httpCommunicator_(httpCommunicator) {
    while (threadCount > 0) {
      auto worker = boost::bind(&EventLoopService::run, this);
      threadGroup_.add_thread(new boost::thread(worker));
      threadCount--;
    }
  }
  virtual ~EventLoopService() {
    working_.reset();  // allow run() to exit
    threadGroup_.join_all();
    io_service_->stop();
  }

 protected:
  // run is called for each thread. It calls io_service.run() and
  // invokes the curl handlers.
  // You only need to invoke this if you want a custom event loop service.
  void run() {
    try {
      io_service_->run();
    } catch (std::exception const& ex) {
      std::cerr << "exception: " << ex.what() << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  // io_service returns a reference to the boost io_service.
  std::shared_ptr<asio_io_service>& io_service() { return io_service_; }
  // httpCommunicator returns a reference to the HTTP communicator.
  std::shared_ptr<http::HttpCommunicator>& httpCommunicator() { return httpCommunicator_; }

 private:
  std::shared_ptr<asio_io_service> io_service_;
  std::shared_ptr<http::HttpCommunicator> httpCommunicator_;
  std::unique_ptr<asio_work> working_;  // Used to keep the io-service alive.
  boost::thread_group threadGroup_;     // Used to join on.
};
/*
// LoopProvider is a meyers singleton so we have private constructors
// call to the class are not thread safe! It is your responsiblity
// to make sure that there is no concurrent access to this class in
// your code!
class LoopProvider {
  LoopProvider(); // private for singleton use getProvider()

public:
  //get LoopProvider Singelton with this function!
  static LoopProvider& getProvider(){
    static LoopProvider provider;
    return provider;
  }

  // get a shared pointer to a http communicator!
  // You need to include the HttpCommunicator.h.
  std::shared_ptr<http::HttpCommunicator> getHttpLoop(){
    return _httpLoop;
  }

  //// io_service modification
  //the service will not be owned by the LoopProvider
  void setAsioService(::boost::asio::io_service*, bool running);
  void setAsioServiceTakeOwnership(::boost::asio::io_service*, bool running);
  // get pointer to the ioservice
  std::shared_ptr<Loop> getAsioLoop();

  // poll / run both loops
  private:
  void poll();
  public:
  void run();
  void resetIoService();

private:
  std::shared_ptr<Loop> _asioLoop;
  std::shared_ptr<http::HttpCommunicator> _httpLoop;
};

// for internal usage
class Loop{
  friend class LoopProvider;
  friend class vst::VstConnection;

public:
  Loop();
  //void run();
  //void ask_to_stop();
  // Run the eventloop. Only return when all work is done or the eventloop is stopped.
  // There is no need to reset the eventloop after this call, unless an exception is thrown.
  void run_ready();
private:
  // Run all handlers that are ready to run without blocking.
  // Only returns when there are no more ready handlers, or or the eventloop is stopped.
  void poll();
  void reset();

private:
  void direct_poll();
  
  // Run the eventloop. Only return when all work is done or the eventloop is stopped.
  void direct_run();
  // Stop the eventloop as soon as possible.
  void direct_stop();
  // Reset the eventloop, so run can function again.
  void direct_reset() { reset(); }

  void setIoService(::boost::asio::io_service * service);
  void setIoServiceTakeOwnership(::boost::asio::io_service* service);
  ::boost::asio::io_service* getIoService();

private:
  std::shared_ptr<::boost::asio::io_service> _serviceSharedPtr;
  ::boost::asio::io_service* _service;
  //std::shared_ptr<Work> _work;
  bool _owning;
  bool _sealed;
  bool _running;
  bool _singleRunMode;
};

// pools the services
// when blocking is true asio will do all outstanding communication and return.
// Otherwise (blocking == false) it will return immediately if there is no
// ready handler so it will not wait for things like epoll on a socket.
// You want to use poll(false) if you are using fuerte with your own event loop.
//inline void poll(){ LoopProvider::getProvider().poll(); };

inline void run(){
  static auto& provider = LoopProvider::getProvider();
  provider.run();
  provider.resetIoService();
};

inline LoopProvider& getProvider(){
  static auto& provider = LoopProvider::getProvider();
  return provider;
};
*/

}}}
#endif

