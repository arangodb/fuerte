#pragma once

#include "internal_types.h"
#include "header.h"

#include <string>
#include <vector>
#include <map>


namespace arangodb { namespace rest { inline namespace v2 {

  class Request {
    public:
      Request(ReHeader&& reHeader = ReHeader()
             ,mapss&& headerStrings = mapss()
             )
             :reHeader(std::move(reHeader))
             ,headerStrings(std::move(headerStrings))
             {}
      ReHeader reHeader;
      mapss headerStrings;
      std::vector<VBuffer> payload;
      std::string user;
      std::string password;

      void addPayload(VSlice const& slice){
        VBuffer buffer;
        buffer.append(slice.startAs<char>(), slice.byteSize());
        payload.push_back(std::move(buffer));
      }
  };

  NetBuffer toNetworkVst(Request const&);
  NetBuffer toNetworkHttp(Request const&);

  Request formNetworkVst(NetBuffer const&);
  Request formNetworkHttp(NetBuffer const&);
}}}
