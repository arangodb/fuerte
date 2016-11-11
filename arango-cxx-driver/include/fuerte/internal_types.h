#pragma once
#include <velocypack/Slice.h>
#include <velocypack/Buffer.h>
#include <velocypack/Builder.h>

#include <map>
#include <vector>
#include <string>
#include <cassert>

namespace arangodb { namespace rest { inline namespace v2 {

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

      enum class ReType
      { Undefined = 0
      , Request = 1
      , Response = 2
      , Authenticaton = 1000
      };

      enum class Transport
      { Undefined = 0
      , Http = 1
      , Vst = 2
      };

      struct Endpoint{
        Transport transport;
        std::string remote_host;
        unsigned remote_port;
        unsigned this_port;
        bool async;
      };

}}}
