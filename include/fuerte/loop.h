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
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef ARANGO_CXX_DRIVER_SERVER
#define ARANGO_CXX_DRIVER_SERVER

#include <utility>
#include <memory>


// run / runWithWork / poll for Loop mapping to ioservice
// free function run with threads / with thread group barrier and work


namespace boost { namespace  asio {
  class io_service;
}}

namespace arangodb { namespace fuerte { inline namespace v1 {

class Work;
class Loop;

namespace vst {
  class VstConnection;
}

namespace http{
  class HttpCommunicator;
}

// need partial rewrite so it can be better integrated in client applications


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
  void poll();
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
  void run_ready();
  void poll();
  void reset();

  void direct_poll();
  void direct_run();
  void direct_stop();
  void direct_reset() { reset(); }

  void setIoService(::boost::asio::io_service * service);
  void setIoServiceTakeOwnership(::boost::asio::io_service* service);
  ::boost::asio::io_service* getIoService();

private:
  std::shared_ptr<::boost::asio::io_service> _serviceSharedPtr;
  ::boost::asio::io_service* _service;
  std::shared_ptr<Work> _work;
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
inline void poll(){ LoopProvider::getProvider().poll(); };

inline void run(){
  static auto& provider = LoopProvider::getProvider();
  provider.run();
  provider.resetIoService();
};

inline LoopProvider& getProvider(){
  static auto& provider = LoopProvider::getProvider();
  return provider;
};

}}}
#endif

