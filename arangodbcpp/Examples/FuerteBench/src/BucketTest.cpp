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
/// Copyright holder is ArangoDB GmbH, Cologne, Ge  `rmany
///
/// @author John Bufton
////////////////////////////////////////////////////////////////////////////////

#include "BucketTest.h"

#include "FuerteBench.h"

BucketTest::BucketTest(const std::string& hostName, const std::string& dbName,
                       const std::string& colName, ConnectionBase::Protocol prot)
    : _server{std::make_shared<Server>(FuerteBench::hostUrl())},
      _database{std::make_shared<Database>(_server, dbName)},
      _collection{std::make_shared<Collection>(_database, colName)} {
  if (!hostName.empty()) {
    _server->setHostUrl(hostName);
  }
  _connection = _server->makeConnection();
  *_connection = prot;
}
