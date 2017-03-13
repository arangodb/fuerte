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

#ifndef ARANGO_CXX_DRIVER_TYPES
#define ARANGO_CXX_DRIVER_TYPES

#include <velocypack/Slice.h>
#include <velocypack/Buffer.h>
#include <velocypack/Builder.h>

#include <map>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>

namespace arangodb { namespace fuerte { inline namespace v1 {

class Request;
class Response;

using Error = std::uint32_t;
using MessageID = uint64_t; //id that identifies a vst::request
using Ticket = MessageID;

using OnSuccessCallback = std::function<void(std::unique_ptr<Request>, std::unique_ptr<Response>)>;
using OnErrorCallback = std::function<void(Error, std::unique_ptr<Request>, std::unique_ptr<Response>)>;

using VBuffer = arangodb::velocypack::Buffer<uint8_t>;
using VSlice = arangodb::velocypack::Slice;
using VBuilder = arangodb::velocypack::Builder;
using VValue = arangodb::velocypack::Value;

using mapss = std::map<std::string,std::string>;
using NetBuffer = std::string;

//move to some other place
class VpackInit {
    std::unique_ptr<arangodb::velocypack::AttributeTranslator> _translator;
  public:
    VpackInit();
};


// -----------------------------------------------------------------------------
// --SECTION--                                         enum class ErrorCondition
// -----------------------------------------------------------------------------

enum class ErrorCondition : Error {
  NoError = 0,
  ErrorCastError = 1,

  ConnectionError = 1000,
  CouldNotConnect = 1001,
  Timeout = 1002,
  VstReadError = 1102,
  VstWriteError =1103,
  VstCanceldDuringReset = 1104,

  CurlError = 3000,

};

inline Error errorToInt(ErrorCondition cond){
  return static_cast<Error>(cond);
}
ErrorCondition intToError(Error integral);
std::string to_string(ErrorCondition error);

// -----------------------------------------------------------------------------
// --SECTION--                                               enum class RestVerb
// -----------------------------------------------------------------------------

enum class RestVerb
{ Illegal = -1
, Delete = 0
, Get = 1
, Post = 2
, Put = 3
, Head = 4
, Patch = 5
, Options = 6
};

RestVerb to_RestVerb(std::string const& val);
std::string to_string(RestVerb type);

// -----------------------------------------------------------------------------
// --SECTION--                                                       MessageType
// -----------------------------------------------------------------------------

enum class MessageType
{ Undefined = 0
, Request = 1
, Response = 2
, ResponseUnfinished = 3
, Authentication = 1000
};

std::string to_string(MessageType type);

// -----------------------------------------------------------------------------
// --SECTION--                                                     TransportType
// -----------------------------------------------------------------------------

enum class TransportType { Undefined = 0, Http = 1, Vst = 2 };
std::string to_string(TransportType type);

// -----------------------------------------------------------------------------
// --SECTION--                                                       ContentType
// -----------------------------------------------------------------------------

enum class ContentType { Unset, Custom, VPack, Dump, Json, Html, Text };
ContentType to_ContentType(std::string const& val);
std::string to_string(ContentType type);

// -----------------------------------------------------------------------------
// --SECTION--                                           ConnectionConfiguration
// -----------------------------------------------------------------------------

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
