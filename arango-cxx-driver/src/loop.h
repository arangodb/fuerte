// internal header
#pragma once
#ifndef ARANGO_CXX_DRIVER_SERVER
#define ARANGO_CXX_DRIVER_SERVER

#include <boost/asio/io_service.hpp>

//#include "asio.h"

#include <utility>
#include <memory>

namespace arangodb { namespace rest { inline namespace v2 {
namespace asio{
  class Loop;
}

class LoopProvider {
  //meyers singleton so we have private constructors
  LoopProvider();

public:
  //get LoopProvider Singelton with this function!
  template<typename ...T>
  LoopProvider& getProvider(T... args){
    static LoopProvider provider(std::forward<T>(args)...);
    return provider;

  }

  std::shared_ptr<asio::Loop> getAsioLoop(){
    return _asioLoop;
  }
  void setAsioLoop(::boost::asio::io_service*);


private:
  std::shared_ptr<asio::Loop> _asioLoop;

};




}}}
#endif

