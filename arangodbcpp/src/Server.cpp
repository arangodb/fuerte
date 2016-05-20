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

#include <curlpp/cURLpp.hpp>
#include <sstream>
#include <algorithm>

#include "arangodbcpp/Server.h"
#include "arangodbcpp/Database.h"
#include "arangodbcpp/HttpConnection.h"

namespace arangodb {

namespace dbinterface {

uint16_t Server::_inst = 0;

Server::Server(const std::string url) {
  if (!_inst) {
    curlpp::initialize();
  }
  ++_inst;
  setHostUrl(url);
}

Server::~Server() {
  --_inst;
  if (!_inst) {
    curlpp::terminate();
  }
}

Connection::SPtr Server::httpConnection() {
  return Connection::SPtr(new HttpConnection());
}

Connection::SPtr Server::vppConnection() { return Connection::SPtr(); }

//
//      Enables the user to set the host url
//
void Server::setHostUrl(const std::string url) {
  typedef std::string::size_type size_type;
  static std::string sep{'/', '/'};
  size_type len = url.find(sep);
  std::string res = url;
  _makeConnection = &Server::httpConnection;
  if (len == std::string::npos) {
    setSrvUrl("http://" + res);
    return;
  }
  std::string prot = url.substr(0, len);
  std::transform(prot.begin(), prot.end(), prot.begin(), ::tolower);
  res = url.substr(len);
  if (prot == "http+ssl:" || prot == "https:") {
    setSrvUrl("https:" + res);
    return;
  }
  if (prot == "vstream+ssl:" || prot == "vstreams:") {
    _makeConnection = &Server::vppConnection;
    setSrvUrl("vstreams:" + res);
    return;
  }
  if (prot == "vstream+tcp:" || prot == "vstream:") {
    _makeConnection = &Server::vppConnection;
    setSrvUrl("vstream:" + res);
    return;
  }
  setSrvUrl("http:" + res);
}

//
//      Configure to request the Arangodb version
//
void Server::version(Connection::SPtr p) {
  Connection& conn = p->reset();
  conn.setUrl(_host + "/_api/version");
  conn.setBuffer();
}

//
//      Configure to request the current default Database
//
void Server::currentDb(Connection::SPtr p) {
  Connection& conn = p->reset();
  conn.setUrl(_host + "/_api/database/current");
  conn.setBuffer();
}

//
//      Configure to request the user Databases available
//
void Server::userDbs(Connection::SPtr p) {
  Connection& conn = p->reset();
  conn.setUrl(_host + "/_api/database/user");
  conn.setBuffer();
}

//
// Configure to request the Databases available
//
void Server::existingDbs(Connection::SPtr p) {
  Connection& conn = p->reset();
  conn.setUrl(_host + "/_api/database");
  conn.setBuffer();
}
}
}
