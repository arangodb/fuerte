#pragma once
#include <fuerte/internal_types.h>
#include <boost/optional.hpp>
namespace arangodb { namespace rest { inline namespace v2 {

  struct ReHeader {
    //the types should all be optional
    ReHeader(ReHeader const&) = delete;
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

  inline std::string headerToHttp(ReHeader const& header){
    return "foo";
  }

  inline VBuffer headerToVst(ReHeader const& header){
    VBuffer  buffer;
    VBuilder builder(buffer);
    builder.openArray();
    builder.add(VValue(header.version.value()));                           // 0
    builder.add(VValue(static_cast<int>(header.type.value())));            // 1
    switch (header.type.get()){
      case ReType::Authenticaton:
        //builder.add(VValue(header.encryption.value()));
        builder.add(VValue(header.user.value()));
        builder.add(VValue(header.password.value()));
        break;

      case ReType::Request:
        builder.add(VValue(header.database.value()));                      // 2
        builder.add(VValue(static_cast<int>(header.requestType.value()))); // 3
        builder.add(VValue(header.requestPath.value()));                   // 4
        if (header.parameter){                                             // 5
          //header.parameter = header_slice.at(5);
        }
        if (header.meta){                                                  // 6
          //header.meta = header_slice.at(6);
        }
        break;

      case ReType::Response:
        builder.add(VValue(header.responseCode.value()));                  // 2
        break;
      default:
        break;
    }
    builder.close();
    return buffer;
  }

  inline ReHeader headerFromHttp(std::string const& body){
    return ReHeader{};
  }

  inline ReHeader headerFromSlice(VSlice const& header_slice){
    assert(header_slice.isArray());
    ReHeader header;

    header.version = header_slice.at(0).getNumber<int>(); //version
    header.type = static_cast<ReType>(header_slice.at(1).getNumber<int>()); //type
    switch (header.type.get()){
      case ReType::Authenticaton:
        //header.encryption = header_slice.at(6); //encryption (plain) should be 2
        header.user = header_slice.at(2).copyString(); //user
        header.user = header_slice.at(3).copyString(); //password
        break;

      case ReType::Request:
        header.database = header_slice.at(2).copyString(); // databse
        header.requestType = static_cast<RestVerb>(header_slice.at(3).getInt()); //rest verb
        header.requestPath = header_slice.at(4).copyString();  // request (path)
        //header.parameter = header_slice.at(5); // params
        //header.parameter = header_slice.at(6); // meta
        break;

      case ReType::Response:
        header.responseCode = header_slice.at(2).getInt();
        break;
      default:
        break;
    }

    return header;
  }
}}}
