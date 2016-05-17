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

#ifndef FUERTE_CONNECTIONURL
#define FUERTE_CONNECTIONURL

#include <string>

namespace arangodb {

namespace dbinterface {

class ConnectionUrl {
 public:
  const std::string& serverUrl() const;
  const std::string& dbName() const;
  void setServerUrl(const std::string& inp);
  void setDbName(const std::string& inp);
  ConnectionUrl& operator+=(const std::string& inp);
  std::string httpUrl() const;

 private:
  std::string _serverUrl;
  std::string _dbName;
  std::string _tailUrl;
};

const ConnectionUrl operator+(const ConnectionUrl& inp, const std::string& add);
ConnectionUrl&& operator+(ConnectionUrl&& inp, const std::string& add);

inline const std::string& ConnectionUrl::serverUrl() const {
  return _serverUrl;
}

inline const std::string& ConnectionUrl::dbName() const { return _dbName; }

inline ConnectionUrl& ConnectionUrl::operator+=(const std::string& inp) {
  _tailUrl += inp;
  return *this;
}

inline void ConnectionUrl::setServerUrl(const std::string& inp) {
  _serverUrl = inp;
}

inline void ConnectionUrl::setDbName(const std::string& inp) { _dbName = inp; }
}
}

#endif
