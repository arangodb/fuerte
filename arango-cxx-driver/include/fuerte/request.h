#pragma once

#include "internal_types.h"
#include "header.h"

#include <boost/optional.hpp>
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
      uint64_t messageid; //generate by some singleton

      void addPayload(VSlice const& slice){
        VBuffer buffer;
        buffer.append(slice.start(), slice.byteSize());
        payload.push_back(std::move(buffer));
      }
  };

  NetBuffer toNetworkVst(Request const&);
  NetBuffer toNetworkHttp(Request const&);

  boost::optional<Request> fromNetworkVst(NetBuffer const&);
  boost::optional<Request> fromNetworkHttp(NetBuffer const&);
}}}
