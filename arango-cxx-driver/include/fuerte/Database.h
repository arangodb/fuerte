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

#ifndef FUERTE_DATABASE_H
#define FUERTE_DATABASE_H 1

#include <memory>

#include <fuerte/Server.h>

namespace arangodb {
namespace dbinterface {
class Server;

class Database {
 public:
  typedef std::shared_ptr<Database> SPtr;
  Database() = delete;
  explicit Database(const Server::SPtr& srv, const std::string& name);
  explicit Database(const Server::SPtr& srv, std::string&& name = "_system");
  virtual ~Database();

  void create(const Connection::SPtr& p, const Connection::VPack& data);
  void create(const Connection::SPtr& conn);
  void remove(const Connection::SPtr& conn);
  void info(const Connection::SPtr& conn);

  Connection::Url databaseUrl(const std::string& tail) const;
  bool hasValidHost() const;
  Database& operator=(const std::string&);
  Database& operator=(std::string&&);
  operator const std::string&() const;

 private:
  static const std::string httpDbApi;

  Server::SPtr _server;
  std::string _name;
};

inline Database::~Database() {}

inline Database::operator const std::string&() const { return _name; }

inline Database& Database::operator=(const std::string& inp) {
  _name = inp;
  return *this;
}

inline Database& Database::operator=(std::string&& inp) {
  _name = inp;
  return *this;
}

inline bool Database::hasValidHost() const { return _server.get() != nullptr; }
}
}

#endif
