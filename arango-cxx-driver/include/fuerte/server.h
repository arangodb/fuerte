// internal header
#pragma once
#ifndef ARANGO_CXX_DRIVER_SERVER
#define ARANGO_CXX_DRIVER_SERVER

#include <utility>
#include <memory>

namespace arangodb { namespace rest { inline namespace v2 {

namespace asio {
  class Loop;
}

class Server {
  Server() = delete;
  Server(bool asio, bool curl, std::shared_ptr<asio::Loop> loop){}
public:
  template<typename ...T>
  Server& getServer(T... args){
    static Server server(std::forward<T>(args)...);
    return server;

  }
private:
  bool _asio_enabled;
  bool _curl_enabled;
  std::shared_ptr<asio::Loop> _loop;



};

}}}
#endif

