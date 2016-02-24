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
#ifndef FUERTE_COLLECTION_H
#define FUERTE_COLLECTION_H
#include <velocypack/Builder.h>

#include "arangodbcpp/Database.h"
#include "arangodbcpp/Connection.h"

namespace arangodb {

namespace dbinterface {

class Collection {
 public:
  typedef uint16_t Options;
  typedef std::shared_ptr<Collection> SPtr;
  enum : Options {
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
  static void httpCreate(const Database::SPtr& pDb,
                         const Connection::SPtr& pCon,
                         const Connection::VPack& body,
                         const Options = Opt_Defaults);
  void httpCreate(const Connection::SPtr& pCon, const Options = Opt_Defaults);
  Connection::VPack httpCreate(bool bSort, const Connection::SPtr& pCon);
  void httpDocs(const Connection::SPtr& pCon, const Options = Opt_Defaults);
  Connection::VPack httpDocs(bool bSort, const Connection::SPtr& pCon);
  void httpDelete(const Connection::SPtr& pCon, const Options = Opt_Defaults);
  Connection::VPack httpDelete(bool bSort, const Connection::SPtr& pCon);
  void httpTruncate(const Connection::SPtr& pCon, const Options = Opt_Defaults);
  Connection::VPack httpTruncate(bool bSort, const Connection::SPtr& pCon);
  std::string docColUrl() const;
  std::string refDocUrl(std::string& key);
  bool hasValidHost() const;
  Collection& operator=(const std::string&);
  Collection& operator=(std::string&&);
  const std::string id() const;
  void addNameAttrib(arangodb::velocypack::Builder& builder);

 private:
  const std::string httpApi() const;

  static std::string httpDocApi;
  static std::string httpColApi;

  std::shared_ptr<Database> _database;
  std::string _name;
};

inline Collection& Collection::operator=(const std::string& inp) {
  _name = inp;
  return *this;
}

inline Collection& Collection::operator=(std::string&& inp) {
  _name = inp;
  return *this;
}

inline const std::string Collection::id() const { return _name; }

inline Connection::VPack Collection::httpCreate(bool bSort,
                                                const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Collection::httpDelete(bool bSort,
                                                const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Collection::httpTruncate(
    bool bSort, const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Collection::httpDocs(bool bSort,
                                              const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}
}
}

#endif  // FUERTE_COLLECTION_H
