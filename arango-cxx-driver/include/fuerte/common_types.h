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
#include <algorithm>

namespace arangodb { namespace fuerte { inline namespace v1 {

class Request;
class Response;

using Error = std::uint32_t;
using Ticket = uint64_t;

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
    VpackInit() : _translator(new arangodb::velocypack::AttributeTranslator){
      _translator->add("_key", uint8_t(1));
      _translator->add("_rev", uint8_t(2));
      _translator->add("_id", uint8_t(3));
      _translator->add("_from", uint8_t(4));
      _translator->add("_to", uint8_t(5));
      _translator->seal();
      arangodb::velocypack::Options::Defaults.attributeTranslator = _translator.get();
    }
};


// -----------------------------------------------------------------------------
// --SECTION--                                         enum class ErrorCondition
// -----------------------------------------------------------------------------

enum class ErrorCondition : Error {
  CouldNotConnect = 1000,
  Timeout = 1001,
  CurlError = 1002
};

// -----------------------------------------------------------------------------
// --SECTION--                                                   class Callbacks
// -----------------------------------------------------------------------------

class Callbacks {
 public:
  Callbacks() {}

  Callbacks(OnSuccessCallback onSuccess, OnErrorCallback onError)
      : _onSuccess(onSuccess), _onError(onError) {}

 public:
  OnSuccessCallback _onSuccess;
  OnErrorCallback _onError;
};

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


inline RestVerb to_RestVerb(std::string& val) {
  std::transform(val.begin(), val.end(), val.begin(), ::tolower );
  auto p = val.c_str();

  if (strcasecmp(p, "delete") == 0) {
    return RestVerb::Delete;
  }

  if (strcasecmp(p, "get") == 0) {
    return RestVerb::Get;
  }

  if (strcasecmp(p, "post") == 0) {
    return RestVerb::Post;
  }

  if (strcasecmp(p, "put") == 0) {
    return RestVerb::Put;
  }

  if (strcasecmp(p, "head") == 0) {
    return RestVerb::Head;
  }

  if (strcasecmp(p, "patch") == 0) {
    return RestVerb::Patch;
  }

  if (strcasecmp(p, "options") == 0) {
    return RestVerb::Options;
  }

  return RestVerb::Illegal;
}

inline RestVerb to_RestVerb(std::string const& valin) {
  std::string val(valin);
  return to_RestVerb(val);
}


inline std::string to_string(RestVerb type) {
  switch (type) {
    case RestVerb::Illegal:
      return "illegal";

    case RestVerb::Delete:
      return "delete";

    case RestVerb::Get:
      return "get";

    case RestVerb::Post:
      return "post";

    case RestVerb::Put:
      return "put";

    case RestVerb::Head:
      return "head";

    case RestVerb::Patch:
      return "patch";

    case RestVerb::Options:
      return "options";
  }

  return "undefined";
}

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

inline std::string to_string(MessageType type) {
  switch (type) {
    case MessageType::Undefined:
      return "undefined";

    case MessageType::Request:
      return "request";

    case MessageType::Response:
      return "response";

    case MessageType::ResponseUnfinished: //needed for vst
      return "unfinised response";

    case MessageType::Authentication:
      return "authentication";
  }

  return "undefined";
}



// -----------------------------------------------------------------------------
// --SECTION--                                                     TransportType
// -----------------------------------------------------------------------------

enum class TransportType { Undefined = 0, Http = 1, Vst = 2 };

inline std::string to_string(TransportType type) {
  switch (type) {
    case TransportType::Undefined:
      return "undefined";

    case TransportType::Http:
      return "http";

    case TransportType::Vst:
      return "vst";
  }

  return "undefined";
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       ContentType
// -----------------------------------------------------------------------------

enum class ContentType { Unset, Custom, VPack, Dump, Json, Html, Text };

inline ContentType to_ContentType(std::string const& val) {
  auto p = val.c_str();

  if (strcasecmp(p, "") == 0) {
    return ContentType::Unset;
  }

  if (val.find("application/json") != std::string::npos) {
    return ContentType::Json;
  }

  //TODO add missing!!!!


  return ContentType::Custom;
}


inline std::string to_string(ContentType type) {
  switch (type) {
    case ContentType::Unset:
      return "unset";

    case ContentType::Custom:
      return "custom";

    case ContentType::VPack:
      return "vpack";

    case ContentType::Dump:
      return "dump";

    case ContentType::Json:
      return "json";

    case ContentType::Html:
      return "html";

    case ContentType::Text:
      return "text";
  }

  return "undefined";
}

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
