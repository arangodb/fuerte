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
namespace asio{
  class Loop;
}
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

  std::shared_ptr<asio::Loop> getAsioLoop();

  std::shared_ptr<http::HttpCommunicator> getHttpLoop(){
    return _httpLoop;
  }

  bool runAsio();
  void stopAsio();
  void pollAsio();

  void poll();

  //the serive will not be owned by the LoopProvider
  void setAsioService(::boost::asio::io_service*, bool running);
  void setAsioServiceTakeOwnership(::boost::asio::io_service*, bool running);
  void* getAsioIoService();

private:
  std::shared_ptr<asio::Loop> _asioLoop;
  std::shared_ptr<http::HttpCommunicator> _httpLoop;
};

inline void poll(){ LoopProvider::getProvider().poll(); };

}}}
#endif

