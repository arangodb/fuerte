#pragma once
#include <fuerte/common_types.h>
#include <vector>
#include <string>
#include <sstream>

namespace arangodb { namespace fuerte { inline namespace v1 {

std::string payloadToString(std::vector<VBuffer> const& payload, std::string name){
  std::stringstream ss;
  ss << name << "(payload)" << std::endl;
  if(!payload.empty()){
    for(auto const& buffer : payload){
      VSlice slice(buffer.data());
      std::string json = slice.toJson();
      ss << json
         << ", " << slice.byteSize()
         << ", " << json.length()
         << std::endl;
    }
  } else {
    ss << "empty" << std::endl;
  }

  return ss.str();
}


}}}
