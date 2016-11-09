#pragma once
#include <fuerte/next/internal_types.h>
namespace arangodb { namespace rest { inline namespace v2 {

  struct ReHeader {
    ReHeader( int version = 0
            , std::string database = "_system"
            , ReType type = ReType::Undefined
            , RestVerb requestType = RestVerb::Get
            , std::string requestPath = ""
            , mapss parameter = mapss()
            , mapss meta = mapss()
            )
            : version(version)
            , database(database)
            , type(type)
            , requestType(requestType)
            , requestPath(requestPath)
            , parameter(parameter)
            , meta(meta)
            {}

    int version;
    std::string database;
    ReType type;
    RestVerb requestType;
    std::string requestPath;
    std::map<std::string, std::string> parameter;
    std::map<std::string, std::string> meta;
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

  inline ReHeader headerFromVst(std::string const& body){
    return ReHeader();
  }
}}}
