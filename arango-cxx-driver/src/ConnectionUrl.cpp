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
/// @author John Bufton
////////////////////////////////////////////////////////////////////////////////

#include "../include/fuerte/ConnectionUrl.h"

namespace arangodb {
namespace dbinterface {
std::string ConnectionUrl::httpUrl() const {
  std::string res = _serverUrl;
  if (!_dbName.empty()) {
    res += "/_db/" + _dbName;
  }
  res += _tailUrl;
  return res;
}

const ConnectionUrl operator+(const ConnectionUrl& inp,
                              const std::string& add) {
  ConnectionUrl res{inp};
  return res += add;
}

ConnectionUrl&& operator+(ConnectionUrl&& inp, const std::string& add) {
  inp += add;
  return std::move(inp);
}
}
}
