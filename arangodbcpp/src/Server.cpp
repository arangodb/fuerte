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
#include <curlpp/cURLpp.hpp>
#include <sstream>

#include "arangodbcpp/Server.h"
#include "arangodbcpp/Database.h"

namespace arangodb {

namespace dbinterface {

uint16_t Server::_inst = 0;

//
//	Create a shared pointer to a server object
//
Server::SPtr Server::create() { return SPtr(new Server()); }

Server::Server() : _host{"http://localhost:8529"} {
  if (!_inst) {
    curlpp::initialize();
  }
  ++_inst;
}

Server::~Server() {
  --_inst;
  if (!_inst) {
    curlpp::terminate();
  }
}

//
//	Enables the user to set the host url
//
void Server::setHostUrl(std::string url, uint16_t port) {
  std::ostringstream os;
  os << "http://" << url << ":" << port;
  _host = os.str();
}

//
//	Configure to request the Arangodb version
//
void Server::httpVersion(Connection::SPtr p, bool bAsync) {
  Connection& conn = *p;
  std::string url{_host + "/_api/version"};
  conn.reset();
  conn.setUrl(url);
  conn.setBuffer();
  conn.setReady(bAsync);
}

//
//	Configure to request the current default Database
//
void Server::httpCurrentDb(Connection::SPtr p, bool bAsync) {
  Connection& conn = *p;
  std::string url{_host + "/_api/database/current"};
  conn.reset();
  conn.setUrl(url);
  conn.setBuffer();
  conn.setReady(bAsync);
}

//
//	Configure to request the user Databases available
//
void Server::httpUserDbs(Connection::SPtr p, bool bAsync) {
  Connection& conn = *p;
  std::string url{_host + "/_api/database/user"};
  conn.reset();
  conn.setUrl(url);
  conn.setBuffer();
  conn.setReady(bAsync);
}

//
//	Configure to request the Databases available
//
void Server::httpExistingDbs(Connection::SPtr p, bool bAsync) {
  Connection& conn = *p;
  std::string url{_host + "/_api/database"};
  conn.reset();
  conn.setUrl(url);
  conn.setBuffer();
  conn.setReady(bAsync);
}
}
}
