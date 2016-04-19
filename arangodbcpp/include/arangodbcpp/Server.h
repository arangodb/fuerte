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

#ifndef FUERTE_SERVER_H
#define FUERTE_SERVER_H 1

#include <memory>
#include <string.h>

#include "arangodbcpp/Connection.h"

namespace arangodb {

namespace dbinterface {

class Database;

class Server {
 public:
  typedef std::shared_ptr<Server> SPtr;
  explicit Server(std::string url = {"localhost"}, uint16_t port = 8529,
                  const bool bSecure = false);
  virtual ~Server();
  void httpVersion(Connection::SPtr conn, bool bAsync = false);
  static Connection::VPack httpVersion(bool bSort, Connection::SPtr conn);
  void httpCurrentDb(Connection::SPtr conn, bool bAsync = false);
  static Connection::VPack httpCurrentDb(bool bSort, Connection::SPtr conn);
  void httpUserDbs(Connection::SPtr conn, bool bAsync = false);
  static Connection::VPack httpUserDbs(bool bSort, Connection::SPtr conn);
  void httpExistingDbs(Connection::SPtr conn, bool bAsync = false);
  static Connection::VPack httpExistingDbs(bool bSort, Connection::SPtr conn);
  void setHostUrl(const std::string url, const uint16_t port,
                  const bool bSecure = false);
  void setHostUrl(const std::string url);
  const std::string& hostUrl() const;

 private:
  static uint16_t _inst;
  std::string _host;
};

inline Connection::VPack Server::httpVersion(bool bSort,
                                             Connection::SPtr conn) {
  return conn->fromJSon(bSort);
}

inline Connection::VPack Server::httpUserDbs(bool bSort,
                                             Connection::SPtr conn) {
  return conn->fromJSon(bSort);
}

inline Connection::VPack Server::httpCurrentDb(bool bSort,
                                               Connection::SPtr conn) {
  return conn->fromJSon(bSort);
}

inline Connection::VPack Server::httpExistingDbs(bool bSort,
                                                 Connection::SPtr conn) {
  return conn->fromJSon(bSort);
}

inline const std::string& Server::hostUrl() const { return _host; }
}
}

#endif
