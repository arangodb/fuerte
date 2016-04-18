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
#ifndef FUERTE_DATABASE_H
#define FUERTE_DATABASE_H

#include <memory>

#include "arangodbcpp/Connection.h"

namespace arangodb {

namespace dbinterface {

class Server;

class Database {
 private:
  typedef std::shared_ptr<Server> ServerPtr;

 public:
  typedef std::shared_ptr<Database> SPtr;
  Database() = delete;
  explicit Database(const ServerPtr& srv, const std::string& name);
  explicit Database(const ServerPtr& srv, std::string&& name = "_system");
  virtual ~Database();
  void httpCreate(const Connection::SPtr& p, const Connection::VPack& data,
                  const bool bAsync);
  void httpCreate(const Connection::SPtr& conn, const bool bAsync = false);
  static Connection::VPack httpCreate(const bool bSort,
                                      const Connection::SPtr& conn);
  void httpDelete(const Connection::SPtr& conn, const bool bAsync = false);
  static Connection::VPack httpDelete(const bool bSort,
                                      const Connection::SPtr& conn);
  void httpInfo(const Connection::SPtr& conn, const bool bAsync = false);
  static Connection::VPack httpInfo(const bool bSort,
                                    const Connection::SPtr& conn);
  std::string databaseUrl() const;
  bool hasValidHost() const;
  Database& operator=(const std::string&);
  Database& operator=(std::string&&);
  operator const std::string&() const;

 private:
  static const std::string httpDbApi;

  ServerPtr _server;
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

inline Connection::VPack Database::httpCreate(const bool bSort,
                                              const Connection::SPtr& conn) {
  return conn->fromJSon(bSort);
}

inline Connection::VPack Database::httpDelete(const bool bSort,
                                              const Connection::SPtr& conn) {
  return conn->fromJSon(bSort);
}

inline Connection::VPack Database::httpInfo(const bool bSort,
                                            const Connection::SPtr& conn) {
  return conn->fromJSon(bSort);
}

inline bool Database::hasValidHost() const { return _server.get() != nullptr; }
}
}

#endif  // FUERTE_DATABASE_H
