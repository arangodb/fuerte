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

#ifndef BUCKETTEST_H
#define BUCKETTEST_H

#include <chrono>

#include <arangodbcpp/Server.h>
#include <arangodbcpp/Database.h>
#include <arangodbcpp/Collection.h>
#include <arangodbcpp/Document.h>

class BucketTest {
 protected:
  typedef arangodb::dbinterface::Server Server;
  typedef arangodb::dbinterface::Database Database;
  typedef arangodb::dbinterface::ConnectionBase ConnectionBase;
  typedef arangodb::dbinterface::Collection Collection;
  typedef arangodb::dbinterface::Document Document;
  typedef ConnectionBase::Protocol Protocol;

 public:
  typedef std::vector<std::string> DocDatas;
  typedef uint32_t TestCount;
  typedef uint16_t LoopCount;

  BucketTest() = delete;
  BucketTest(const std::string& hostName, const std::string& dbName,
             const std::string& colName, ConnectionBase::Protocol prot);

  std::chrono::microseconds duration() const;
  TestCount successful() const;
  TestCount failed() const;

  virtual void operator()(std::atomic_bool& bWait, LoopCount loops) = 0;

 protected:
  Server::SPtr _server;
  Database::SPtr _database;
  Collection::SPtr _collection;
  ConnectionBase::SPtr _connection;

  std::chrono::microseconds _duration;
  TestCount _failed;
  TestCount _successful;
};

inline std::chrono::microseconds BucketTest::duration() const {
  return _duration;
}

inline BucketTest::TestCount BucketTest::successful() const {
  return _successful;
}

inline BucketTest::TestCount BucketTest::failed() const { return _failed; }

#endif