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
#include <fuerte/helper.h>
#include <string.h>
#include <stdexcept>
#include <sstream>

#include <velocypack/Iterator.h>

namespace arangodb { namespace fuerte { inline namespace v1 {

StringMap sliceToStringMap(VSlice const& slice){
  StringMap rv;
  assert(slice.isObject());
  for(auto const& it : ::arangodb::velocypack::ObjectIterator(slice)){
    rv.insert({it.key.copyString(), it.value.copyString()});
  }
  return rv;
}

std::string to_string(VSlice const& slice){
  std::stringstream ss;
  try {
    std::string json = slice.toJson();
    ss << json
       << ", " << slice.byteSize()
       << ", " << json.length();
  } catch(std::exception& e) {
    ss << "slice to string failed with: " <<e.what();
  }
  ss << std::endl;

  return ss.str();
}


std::string to_string(std::vector<VSlice> const& slices){
  std::stringstream ss;
  if(!slices.empty()){
    for(auto const& slice : slices){
      ss << to_string(slice);
    }
  } else {
    ss << "empty";
  }
  ss << "\n";
  return ss.str();
}

//message is not const because message.slices is not
std::string to_string(Message& message){
  std::stringstream ss;
  ss << "\n#### Message #####################################\n";
  ss << "Id:" <<  message.messageID << "\n";
  ss << "Header:\n";
  ss << to_string(message.header);
  ss << "\nBody:\n";
  if(message.contentType() == ContentType::VPack){
    ss << to_string(message.slices());
  } else {
    ss << message.payloadAsString();
  }
  ss << "\n";
  ss << "##################################################\n";
  return ss.str();
}

}}}
