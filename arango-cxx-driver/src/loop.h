// internal header
#pragma once
#ifndef ARANGO_CXX_DRIVER_SERVER
#define ARANGO_CXX_DRIVER_SERVER

#include <utility>
#include <memory>

namespace arangodb { namespace rest { inline namespace v2 {
namespace asio{
  class Loop;
}

class LoopProvider {
  LoopProvider() = delete;
  LoopProvider(std::shared_ptr<asio::Loop> loop){}
public:
  template<typename ...T>
  LoopProvider& getProvider(T... args){
    static LoopProvider provider(std::forward<T>(args)...);
    return provider;

  }
private:



};

}}}
#endif

