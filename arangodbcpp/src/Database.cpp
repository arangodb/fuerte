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
#include "arangodbcpp/Collection.h"
#include "arangodbcpp/Connection.h"

namespace arangodb {

namespace dbinterface {

Database::Database(Server::SPtr srv, std::string name)
    : _server{srv}, _name(name) {}

//
//	Get the core database url
//
std::string Database::databaseUrl() const {
  std::ostringstream os;
  os << _server->hostUrl();
  if (!_name.empty()) {
    os << "/_db/" << _name;
  }
  return os.str();
}

//
//	Configure to create a Database using the configured name
//
void Database::httpCreate(Connection::SPtr p, bool bAsync) {
  std::ostringstream os;
  Connection& conn = *p;
  Connection::HttpHeaderList headers;
  conn.reset();
  conn.setJsonContent(headers);
  conn.setHeaderOpts(headers);
  os << _server->hostUrl() << "/_api/database";
  conn.setUrl(os.str());
  os.str("");
  os << "{ \"name\":\"" << _name << "\" }";
  conn.setPostReq();
  conn.setPostField(os.str());
  conn.setBuffer();
  conn.setReady(bAsync);
}

//
//	Configure to drop a Database using the configured name
//
void Database::httpDelete(Connection::SPtr p, bool bAsync) {
  std::ostringstream os;
  Connection& conn = *p;
  conn.reset();
  os << _server->hostUrl() << "/_api/database/" << _name;
  conn.setUrl(os.str());
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setReady(bAsync);
}
}
}
