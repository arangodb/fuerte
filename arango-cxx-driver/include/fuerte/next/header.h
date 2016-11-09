#pragma once
#include <fuerte/next/internal_types.h>
namespace arangodb { namespace rest { inline namespace v2 {

  struct ReHeader {
    ReHeader(ReHeader const&) = delete;
    ReHeader(ReHeader&&) = default;
    ReHeader( int version = 0
            , ReType type = ReType::Undefined
            , std::string database = "_system"
            , RestVerb requestType = RestVerb::Get
            , std::string requestPath = ""
            , mapss parameter = mapss()
            , mapss meta = mapss()
            , std::string user = ""
            , std::string password = ""
            )
            : version(version)
            , type(type)
            , database(database)
            , requestType(requestType)
            , requestPath(requestPath)
            , parameter(parameter)
            , meta(meta)
            , user(std::move(user))
            , password(std::move(password))
            {}

    int version;
    ReType type;
    std::string database;
    RestVerb requestType;
    std::string requestPath;
    std::map<std::string, std::string> parameter;
    std::map<std::string, std::string> meta;
    std::string user;
    std::string password;
  };

  inline std::string headerToHttp(ReHeader const& header){
    return "foo";
  }

  inline std::string headerToVst(ReHeader const& header){
    return "foo";
  }

  inline ReHeader headerFromHttp(std::string const& body){
    return ReHeader();
  }

  inline ReHeader headerFromSlice(VSlice const& header_slice){
    assert(header_slice.isArray());
    ReHeader header;

    header.version = header_slice.at(0).getNumber<int>();
    header.type = static_cast<ReType>(header_slice.at(1).getNumber<int>());
    if (header.type == ReType::Authenticaton){
      header.user = header_slice.at(2).copyString();
      header.user = header_slice.at(3).copyString();
    } else {
      header.database = header_slice.at(2).copyString();
      header.requestType = static_cast<RestVerb>(header_slice.at(3).getInt());
      header.requestPath = header_slice.at(4).copyString();  // request (path)
      //header.parameter = header_slice.at(5);
      //header.parameter = header_slice.at(6);
    }


    return header;
  }
}}}
