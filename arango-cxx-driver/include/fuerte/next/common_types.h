#pragma once
#ifndef ARANGO_CXX_DRIVER_COMMON_TYPES
#define ARANGO_CXX_DRIVER_COMMON_TYPES

#include <string>

namespace arangocxx {
  namespace detail {

    enum class ConnectionType {
      http, vst
    };

    struct ConnectionConfiguration {
      ConnectionConfiguration()
        : _connType(ConnectionType::vst)
        , _ssl(true)
        , _async(false)
        , _host("localhost")
        , _user("root")
        , _password("foppels")
        , _maxChunkSize(5000ul) // in bytes
        {}

      ConnectionType _connType; // vst or http
      bool _ssl;
      bool _async;
      std::string _host;
      std::string _port;
      std::string _user;
      std::string _password;
      std::size_t _maxChunkSize;
    };

  }
}
#endif

