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
#ifndef COLLECTION_H
#define COLLECTION_H

#include "arangodbcpp/Connection.h"
#include "arangodbcpp/DocOptions.h"

namespace arangodb {

namespace dbinterface {

class Database;

class Collection {
 public:
  typedef uint16_t Options;
  typedef std::shared_ptr<Collection> SPtr;
  enum {
    Opt_Defaults = 0,
    Opt_RunSync = 0,
    Opt_RunAsync = 1,
    Opt_ListPath = 0,
    Opt_ListId = 2,
    Opt_ListKey = 4,
    Opt_ListMask = Opt_ListId | Opt_ListKey
  };
  Collection() = delete;
  explicit Collection(const std::shared_ptr<Database>& db,
                      const std::string& id);
  explicit Collection(const std::shared_ptr<Database>& db,
                      std::string&& id = "new-collection");
  ~Collection();
  void httpCreateDoc(const Connection::SPtr p, const DocOptions& opts,
                     const Connection::VPack data);
  Connection::VPack httpCreateDoc(bool bSort, const Connection::SPtr conn);
  void httpCreate(const Connection::SPtr conn, const Options = Opt_Defaults);
  Connection::VPack httpCreate(bool bSort, const Connection::SPtr conn);
  void httpDocs(const Connection::SPtr conn, const Options = Opt_Defaults);
  Connection::VPack httpDocs(bool bSort, const Connection::SPtr conn);
  void httpDelete(const Connection::SPtr conn, const Options = Opt_Defaults);
  Connection::VPack httpDelete(bool bSort, const Connection::SPtr conn);
  std::string selectUrl();
  std::string refDocUrl(std::string key);
  bool hasValidHost() const;
  Collection& operator=(const std::string&);
  Collection& operator=(std::string&&);
  const std::string id() const;

 private:
  std::shared_ptr<Database> _database;
  std::string _id;
};

inline Collection& Collection::operator=(const std::string& inp) {
  _id = inp;
  return *this;
}

inline Collection& Collection::operator=(std::string&& inp) {
  _id = inp;
  return *this;
}

inline const std::string Collection::id() const { return _id; }

inline Connection::VPack Collection::httpCreate(bool bSort,
                                                const Connection::SPtr conn) {
  return conn->fromJSon(bSort);
}

inline Connection::VPack Collection::httpCreateDoc(
    bool bSort, const Connection::SPtr conn) {
  return conn->fromJSon(bSort);
}

inline Connection::VPack Collection::httpDelete(bool bSort,
                                                const Connection::SPtr conn) {
  return conn->fromJSon(bSort);
}

inline Connection::VPack Collection::httpDocs(bool bSort,
                                              const Connection::SPtr conn) {
  return conn->fromJSon(bSort);
}
}
}

#endif  // COLLECTION_H
