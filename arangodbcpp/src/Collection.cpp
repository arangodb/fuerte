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

namespace arangodb {

namespace dbinterface {

Collection::Collection(const Database::SPtr& db, const std::string& id)
    : _database(db), _id(id) {}

Collection::Collection(const Database::SPtr& db, std::string&& id)
    : _database(db), _id(id) {}

Collection::~Collection() {}

//
//	Creates the base url required for selecting this collection
//  for various Document operations
//
std::string Collection::selectUrl() {
  std::string url{_database->databaseUrl()};
  url += "/_api/document";
  if (!_id.empty()) {
    url += "?collection=" + _id;
  }
  return url;
}

bool Collection::hasValidHost() const { return _database->hasValidHost(); }

//
//		Creates the base url required to get and delete a Document in
//		the Database Collection configured using the key value passed in
//
std::string Collection::refDocUrl(std::string key) {
  std::string url{_database->databaseUrl() + "/_api/document"};
  if (!_id.empty()) {
    url += "/" + _id;
  }
  if (!key.empty()) {
    url += "/" + key;
  }
  return url;
}

void Collection::httpDocs(const Connection::SPtr p, const Options opts) {
  Connection& conn = *p;
  std::string url{selectUrl()};
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

void Collection::httpCreateDoc(const Connection::SPtr p, const DocOptions& opts,
                               const Connection::VPack data) {
  Connection& conn = *p;
  Connection::HttpHeaderList headers;
  conn.reset();
  conn.setUrl(selectUrl());
  conn.setJsonContent(headers);
  conn.setHeaderOpts(headers);
  conn.setPostField(data);
  conn.setPostReq();
  conn.setBuffer();
  conn.setReady((opts.opts() & DocOptions::Opt_RunAsync) != 0);
}

//
//		Configure to create an empty Collection using the configured
//Database
//		and Collection name
//
void Collection::httpCreate(const Connection::SPtr p, const Options opts) {
  Connection& conn = *p;
  Connection::HttpHeaderList headers;
  std::string val{_database->databaseUrl() + "/_api/collection"};
  conn.reset();
  conn.setJsonContent(headers);
  conn.setHeaderOpts(headers);
  conn.setUrl(val);
  val = "{ \"name\":\"" + _id + "\" }";
  conn.setPostField(val);
  conn.setBuffer();
  conn.setReady((opts & Opt_RunAsync) != 0);
}

//
//		Configure to delete a Collection using the configured Database
//		and Collection name
//
void Collection::httpDelete(const Connection::SPtr p, const Options opts) {
  Connection& conn = *p;
  std::string url{_database->databaseUrl() + "/_api/collection/" + _id};
  conn.reset();
  conn.setDeleteReq();
  conn.setUrl(url);
  conn.setBuffer();
  conn.setReady((opts & Opt_RunAsync) != 0);
}
}
}
