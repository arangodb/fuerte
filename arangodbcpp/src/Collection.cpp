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
#include "arangodbcpp/Database.h"
#include "arangodbcpp/Collection.h"
#include "arangodbcpp/Document.h"

namespace arangodb {

namespace dbinterface {

Collection::Collection(const Database::SPtr& db, const std::string& id)
    : _database(db), _name(id) {}

Collection::Collection(const Database::SPtr& db, std::string&& id)
    : _database(db), _name(id) {}

Collection::~Collection() {}

std::string Collection::httpDocApi{"/_api/document"};
std::string Collection::httpColApi{"/_api/collection"};

//
// Creates the base url required for selecting this collection
// for various Document operations
//
std::string Collection::docColUrl() const {
  return _database->databaseUrl() + httpDocApi + "?collection=" + _name;
}

//
// Creates the base url required to get and delete a Document
//
std::string Collection::refDocUrl(std::string& key) {
  return std::string{_database->databaseUrl() + httpDocApi + '/' + _name + '/' +
                     key};
}

const std::string Collection::httpApi() const {
  return _database->databaseUrl() + httpColApi;
}

void Collection::addNameAttrib(arangodb::velocypack::Builder& builder) {
  builder.add("name", arangodb::velocypack::Value(_name));
}

bool Collection::hasValidHost() const { return _database->hasValidHost(); }

void Collection::httpDocs(const Connection::SPtr& pCon, const Options opts) {
  Connection& conn = *pCon;
  std::string url{docColUrl()};
  conn.reset();
  switch (opts & Opt_ListMask) {
    case Opt_ListId: {
      url += "&type=id";
      break;
    }
    case Opt_ListKey: {
      url += "&type=key";
      break;
    }
    default:;
  }
  conn.setUrl(url);
  conn.setBuffer();
  conn.setReady((opts & Opt_RunAsync) != 0);
}

//
// Configure to create an empty Collection using the configured
// Database.
//
void Collection::httpCreate(const Database::SPtr& pDb,
                            const Connection::SPtr& pCon,
                            const Connection::VPack& config,
                            const Options opts) {
  Connection& conn = *pCon;
  conn.reset();
  conn.setUrl(pDb->databaseUrl() + httpColApi);
  conn.setPostField(Connection::json(config, false));
  conn.setBuffer();
  conn.setReady((opts & Opt_RunAsync) != 0);
}

//
// Configure to create an empty Collection using the configured
// Database and Collection name
//
void Collection::httpCreate(const Connection::SPtr& pCon, const Options opts) {
  Connection& conn = *pCon;
  conn.reset();
  conn.setUrl(httpApi());
  conn.setPostField("{ \"name\":\"" + _name + "\" }");
  conn.setBuffer();
  conn.setReady((opts & Opt_RunAsync) != 0);
}

//
// Configure to delete a Collection using the configured Database
// and Collection name
//
void Collection::httpDelete(const Connection::SPtr& pCon, const Options opts) {
  Connection& conn = *pCon;
  conn.reset();
  conn.setDeleteReq();
  conn.setUrl(httpApi() + '/' + _name);
  conn.setBuffer();
  conn.setReady((opts & Opt_RunAsync) != 0);
}

void Collection::httpTruncate(const Connection::SPtr& pCon,
                              const Options opts) {
  Connection& conn = *pCon;
  conn.reset();
  conn.setPutReq();
  conn.setUrl(httpApi() + '/' + _name + "/truncate");
  conn.setBuffer();
  conn.setReady((opts & Opt_RunAsync) != 0);
}
}
}
