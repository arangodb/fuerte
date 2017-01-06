// internal header
#pragma once
#ifndef ARANGO_CXX_DRIVER_SERVER
#define ARANGO_CXX_DRIVER_SERVER

#include <boost/asio/io_service.hpp>

//#include "asio.h"

#include <utility>
#include <memory>

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

  std::shared_ptr<asio::Loop> getAsioLoop(){
    _sealed = true;
    return _asioLoop;
  }

  std::shared_ptr<http::HttpCommunicator> getHttpLoop(){
    return _httpLoop;
  }

  //the serive will not be owned by the LoopProvider
  void setAsioLoop(::boost::asio::io_service*);
  void setAsioLoopTakeOwnership(::boost::asio::io_service*);


private:
  std::shared_ptr<asio::Loop> _asioLoop;
  std::shared_ptr<http::HttpCommunicator> _httpLoop;
  bool _sealed;

};

void runHttpLoopInThisThread();


}}}
#endif

