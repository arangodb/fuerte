// internal header
#pragma once
#ifndef ARANGO_CXX_DRIVER_SERVER
#define ARANGO_CXX_DRIVER_SERVER

#include <utility>
#include <memory>

namespace boost { namespace  asio {
  class io_service;
}}

namespace arangodb { namespace fuerte { inline namespace v1 {

class Loop;

namespace http{
  class HttpCommunicator;
}

class LoopProvider {
  //meyers singleton so we have private constructors
  LoopProvider();

public:
  //get LoopProvider Singelton with this function!
  static LoopProvider& getProvider(){
    static LoopProvider provider;
    return provider;
  }

  std::shared_ptr<Loop> getAsioLoop();

  std::shared_ptr<http::HttpCommunicator> getHttpLoop(){
    return _httpLoop;
  }

  bool runAsio();
  void stopAsio();
  void pollAsio(bool block);
  bool isAsioPolling();

  void poll(bool block);

  //the service will not be owned by the LoopProvider
  void setAsioService(::boost::asio::io_service*, bool running);
  void setAsioServiceTakeOwnership(::boost::asio::io_service*, bool running);
  void* getAsioIoService();

private:
  std::shared_ptr<Loop> _asioLoop;
  std::shared_ptr<http::HttpCommunicator> _httpLoop;
};


// pools the services
// when blocking is true asio will do all outstanding communication and return.
// Otherwise (blocking == false) it will return immediately if there is no
// ready handler so it will not wait for things like epoll on a socket.
// You want to use poll(false) if you are using fuerte with your own event loop.
inline void poll(bool block){ LoopProvider::getProvider().poll(block); };

}}}
#endif

