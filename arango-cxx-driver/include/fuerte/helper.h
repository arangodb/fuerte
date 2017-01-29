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
#pragma once
#ifndef ARANGO_CXX_DRIVER_HELPER
#define ARANGO_CXX_DRIVER_HELPER
#include <string>
#include <vector>
#include <fuerte/types.h>

namespace arangodb { namespace fuerte { inline namespace v1 {

std::string to_string(VSlice const& slice);
std::string to_string(std::vector<VSlice> const& payload);
mapss sliceToStringMap(VSlice const&);

namespace http {
  std::string urlDecode(std::string const& str);
  std::string urlEncode(char const* src, size_t const len);
  std::string urlEncode(char const* src);

  inline std::string urlEncode(std::string const& str) {
    return urlEncode(str.c_str(), str.size());
  }
}
}}}
#endif
