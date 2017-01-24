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

class Work;
class Loop;

namespace vst {
  class VstConnection;
}

namespace http{
  class HttpCommunicator;
}

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
  void* getAsioIoService();
  std::shared_ptr<Loop> getAsioLoop();

  // run asio loop / poll vst connection
  bool runAsio();
  void stopAsio();
  void pollAsio(bool block);
  bool isAsioPolling();

  // poll both loops
  void poll(bool block);

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
  bool run();
  void ask_to_stop();
  void poll(bool block);

private:
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
  bool _pollMode;
};

// pools the services
// when blocking is true asio will do all outstanding communication and return.
// Otherwise (blocking == false) it will return immediately if there is no
// ready handler so it will not wait for things like epoll on a socket.
// You want to use poll(false) if you are using fuerte with your own event loop.
inline void poll(bool block){ LoopProvider::getProvider().poll(block); };

}}}
#endif

