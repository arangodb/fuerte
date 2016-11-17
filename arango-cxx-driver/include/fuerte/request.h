#pragma once

#include "common_types.h"

#include <boost/optional.hpp>
#include <string>
#include <vector>
#include <map>


namespace arangodb { namespace rest { inline namespace v2 {

  struct ReHeader {
    //the types should all be optional
    ReHeader(ReHeader const&) = default;
    ReHeader() = default;
    ReHeader(ReHeader&&) = default;

    ::boost::optional<int> version;
    ::boost::optional<ReType> type;
    ::boost::optional<unsigned> responseCode;
    ::boost::optional<std::string> database;
    ::boost::optional<RestVerb> requestType;
    ::boost::optional<std::string> requestPath;
    ::boost::optional<mapss> parameter;
    ::boost::optional<mapss> meta;
    ::boost::optional<std::string> user;
    ::boost::optional<std::string> password;
  };

  inline std::string headerToHttp(ReHeader const& header);
  inline VBuffer headerToVst(ReHeader const& header);
  inline ReHeader headerFromHttp(std::string const& body);
  inline ReHeader headerFromSlice(VSlice const& header_slice);

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

  //boost::optional<Request> fromNetworkVst(NetBuffer const&);
  //boost::optional<Request> fromNetworkHttp(NetBuffer const&);
}}}
