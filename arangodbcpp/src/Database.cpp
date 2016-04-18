////////////////////////////////////////////////////////////////////////////////
/// @brief C++ Library to interface to Arangodb.
///
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
/// @author Copyright 2016, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////
#include <sstream>

#include "arangodbcpp/Server.h"
#include "arangodbcpp/Database.h"

namespace arangodb {

namespace dbinterface {

const std::string Database::httpDbApi{"/_api/database"};

Database::Database(const Server::SPtr& srv, const std::string& name)
    : _server{srv}, _name(name) {}

Database::Database(const Server::SPtr& srv, std::string&& name)
    : _server{srv}, _name(name) {}

//
//	Get the core database url
//
std::string Database::databaseUrl() const {
  return _server->hostUrl() + "/_db/" + _name;
}

//
// Configure to create a Database using the VPack configuration data
//
void Database::httpCreate(const Connection::SPtr& p,
                          const Connection::VPack& data, const bool bAsync) {
  std::string val{_server->hostUrl() + httpDbApi};
  Connection& conn = p->reset();
  conn.setUrl(val);
  conn.setPostReq();
  conn.setPostField(Connection::json(data));
  conn.setBuffer();
  conn.setSync(bAsync);
}

//
// Configure to create a Database using the configured name
//
void Database::httpCreate(const Connection::SPtr& p, const bool bAsync) {
  std::string val{_server->hostUrl() + httpDbApi};
  Connection& conn = p->reset();
  conn.setUrl(val);
  val = "{ \"name\" : \"" + _name + "\" }";
  conn.setPostField(val);
  conn.setPostReq();
  conn.setBuffer();
  conn.setSync(bAsync);
}

//
// Configure to get info on the current Database
//
void Database::httpInfo(const Connection::SPtr& p, const bool bAsync) {
  std::string url{_server->hostUrl() + httpDbApi + "/current"};
  Connection& conn = p->reset();
  conn.setUrl(url);
  conn.setGetReq();
  conn.setBuffer();
  conn.setSync(bAsync);
}

//
// Configure to drop a Database using the configured name
//
void Database::httpDelete(const Connection::SPtr& p, const bool bAsync) {
  std::string url{_server->hostUrl() + httpDbApi + '/' + _name};
  Connection& conn = p->reset();
  conn.setUrl(url);
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setSync(bAsync);
}
}
}
