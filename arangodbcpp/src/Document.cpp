////////////////////////////////////////////////////////////////////////////////
/// @brief C++ Library to interface to Arangodb.
///
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
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

#include "arangodbcpp/Database.h"
#include "arangodbcpp/Collection.h"
#include "arangodbcpp/Document.h"

namespace arangodb {

namespace dbinterface {

Document::Document(std::string inp) : _key(inp) {}

Document::~Document() {}

//
// Configure to create a new empty document with the set key name at the
// Database/Collection
// location determined by the pCol parameter
//
void Document::httpCreate(Collection::SPtr pCol, Connection::SPtr pCon,
                          bool bAsync) {
  Connection& conn = *pCon;
  std::ostringstream os;
  conn.reset();
  conn.setUrl(pCol->createDocUrl());
  os << "{ \"_key\":\"" << _key << "\" }";
  conn.setJsonContent();
  conn.setPostField(os.str());
  conn.setPostReq();
  conn.setBuffer();
  conn.setReady(bAsync);
}

//
//	Configure to delete a document with the set key name in the
//	Database/Collection
//	location determined by the pCol parameter
//
void Document::httpDelete(Collection::SPtr pCol, Connection::SPtr pCon,
                          bool bAsync) {
  Connection& conn = *pCon;
  std::ostringstream os;
  conn.reset();
  conn.setUrl(pCol->refDocUrl(_key));
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setReady(bAsync);
}

//
//  Configure to get a document with the set key name in the
//	Database/Collection
//	location determined by the pCol parameter
//
void Document::httpGet(Collection::SPtr pCol, Connection::SPtr pCon,
                       bool bAsync) {
  Connection& conn = *pCon;
  std::ostringstream os;
  conn.reset();
  conn.setUrl(pCol->refDocUrl(_key));
  conn.setBuffer();
  conn.setReady(bAsync);
}
}
}