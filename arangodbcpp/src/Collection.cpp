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
#include <sstream>

#include "arangodbcpp/Database.h"
#include "arangodbcpp/Collection.h"

namespace arangodb {

namespace dbinterface {

Collection::Collection(Database::SPtr db, std::string nm)
    : _database(db), _id(nm) {}

Collection::~Collection() {}

/**

        Creates the base url required for creating a Document in the
        Database Collection configured

*/
std::string Collection::createDocUrl() {
  std::ostringstream os;
  os << _database->getDatabaseUrl() << "/_api/document";
  os << "?collection=" << _id;
  return os.str();
}

/**

        Creates the base url required to get and delete a Document in the
        Database Collection configured using the key value passed in

*/
std::string Collection::refDocUrl(std::string key) {
  std::ostringstream os;
  os << _database->getDatabaseUrl();
  os << "/_api/document/" << _id << "/" << key;
  return os.str();
}

/**

        Configure to create a Collection using the configured Database
        and Collection name

*/
void Collection::httpCreate(Connection::SPtr p, bool bAsync) {
  Connection& conn = *p;
  std::ostringstream os;
  conn.reset();
  conn.setJsonContent();
  os << _database->getDatabaseUrl() << "/_api/collection";
  conn.setUrl(os.str());
  os.str("");
  os << "{ \"name\":\"" << _id << "\" }";
  conn.setPostField(os.str());
  conn.setBuffer();
  conn.setReady(bAsync);
}

/**

        Configure to delete a Collection using the configured Database
        and Collection name

*/
void Collection::httpDelete(Connection::SPtr p, bool bAsync) {
  Connection& conn = *p;
  std::ostringstream os;
  conn.reset();
  os << _database->getDatabaseUrl() << "/_api/collection/" << _id;
  conn.setDeleteReq();
  conn.setUrl(os.str());
  conn.setBuffer();
  conn.setReady(bAsync);
}
}
}
