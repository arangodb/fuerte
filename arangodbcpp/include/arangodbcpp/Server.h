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

#include "arangodbcpp/ConnectionBase.h"

namespace arangodb {

namespace dbinterface {

class Server {
 public:
  typedef std::shared_ptr<Server> SPtr;
  explicit Server(std::string url = {"http://127.0.0.1:8529"});
  virtual ~Server();

  void version(ConnectionBase::SPtr conn);
  void currentDb(ConnectionBase::SPtr conn);
  void userDbs(ConnectionBase::SPtr conn);
  void existingDbs(ConnectionBase::SPtr conn);
  void setHostUrl(const std::string url);
  const ConnectionBase::Url& hostUrl() const;
  ConnectionBase::SPtr makeConnection() const;

 private:
  typedef ConnectionBase::SPtr (*ConFnc)();
  void setSrvUrl(const std::string& url);
  static ConnectionBase::SPtr httpConnection();
  static ConnectionBase::SPtr vppConnection();

  static uint16_t _inst;
  ConnectionBase::Url _host;
  ConFnc _makeConnection;
};

inline ConnectionBase::SPtr Server::makeConnection() const {
  return (*_makeConnection)();
}

#ifdef FUERTE_CONNECTIONURL

inline void Server::setSrvUrl(const std::string& url) {
  _host.setServerUrl(url);
}

#else

inline void Server::setSrvUrl(const std::string& url) { _host = url; }

#endif

inline const ConnectionBase::Url& Server::hostUrl() const { return _host; }
}
}

#endif
