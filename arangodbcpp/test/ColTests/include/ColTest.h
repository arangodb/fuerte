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

#ifndef COLTEST_H
#define COLTEST_H

#include <gtest/gtest.h>

#include "arangodbcpp/Server.h"
#include "arangodbcpp/Database.h"
#include "arangodbcpp/Collection.h"

class ColTest : public testing::Test {
 public:
  typedef arangodb::dbinterface::ConnectionBase ConnectionBase;
  typedef arangodb::dbinterface::Database Database;
  typedef arangodb::dbinterface::Server Server;
  typedef arangodb::dbinterface::Collection Collection;
  ColTest();
  virtual ~ColTest();

 protected:
  void test1();
  void test2();

 private:
  void generalTest(const ConnectionBase::VPack (ColTest::*fn)(), uint64_t code);
  void createTest();
  void collectionsTest();
  void deleteTest();
  void renameTest(const std::string& name);
  void aboutTest();
  void countTest(const uint16_t cnt);
  void truncateTest();
  void docsTest(const std::string& docKey,
                const Collection::Options opts = Collection::Options{});
  void commonTest(const ConnectionBase::VPack (ColTest::*pTestFnc)(),
                  std::string&& scope);
  const ConnectionBase::VPack createDatabase();
  const ConnectionBase::VPack deleteDatabase();
  const ConnectionBase::VPack createCollection();
  const ConnectionBase::VPack deleteCollection();
  const ConnectionBase::VPack truncateCollection();
  const ConnectionBase::VPack docCount();
  const ConnectionBase::VPack collections();
  const ConnectionBase::VPack collectionProps();
  const ConnectionBase::VPack aboutCollection();
  const ConnectionBase::VPack renameCollection(const std::string& name);
  const ConnectionBase::VPack addDocument(const std::string& name);
  const ConnectionBase::VPack collectionDocs(const Collection::Options opts);

  Server::SPtr _pSrv;
  Database::SPtr _pDb;
  Collection::SPtr _pCol;
  ConnectionBase::SPtr _pCon;
};

inline void ColTest::createTest() {
  commonTest(&ColTest::createCollection, "Create");
}

inline void ColTest::collectionsTest() {
  commonTest(&ColTest::collections, "Collections");
}

inline void ColTest::aboutTest() {
  commonTest(&ColTest::aboutCollection, "About");
}

inline void ColTest::truncateTest() {
  commonTest(&ColTest::truncateCollection, "Truncate");
}

#endif  // COLTEST_H
