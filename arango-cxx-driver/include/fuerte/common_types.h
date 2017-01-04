#pragma once
#ifndef ARANGO_CXX_DRIVER_COMMON_TYPES
#define ARANGO_CXX_DRIVER_COMMON_TYPES

#include <velocypack/Slice.h>
#include <velocypack/Buffer.h>
#include <velocypack/Builder.h>

#include <map>
#include <vector>
#include <string>
#include <cassert>

namespace arangodb { namespace fuerte { inline namespace v1 {

using VBuffer = arangodb::velocypack::Buffer<uint8_t>;
using VSlice = arangodb::velocypack::Slice;
using VBuilder = arangodb::velocypack::Builder;
using VValue = arangodb::velocypack::Value;
using mapss = std::map<std::string,std::string>;
using NetBuffer = std::string;

enum class RestVerb
{ Delete = 0
, Get = 1
, Post = 2
, Put = 3
, Head = 4
, Patch = 5
, Optons = 6
};

enum class MessageType
{ Undefined = 0
, Request = 1
, Response = 2
, ResponseUnfinished = 3
, Authenticaton = 1000
};

enum class TransportType
{ Undefined = 0
, Http = 1
, Vst = 2
};

namespace detail {
  struct ConnectionConfiguration {
    ConnectionConfiguration()
      : _connType(TransportType::Vst)
      , _ssl(true)
      , _async(false)
      , _host("localhost")
      , _user("root")
      , _password("foppels")
      , _maxChunkSize(5000ul) // in bytes
      {}

    TransportType _connType; // vst or http
    bool _ssl;
    bool _async;
    std::string _host;
    std::string _port;
    std::string _user;
    std::string _password;
    std::size_t _maxChunkSize;
  };

}

}}}
#endif
