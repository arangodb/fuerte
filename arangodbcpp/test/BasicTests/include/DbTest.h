////////////////////////////////////////////////////////////////////////////////
/// @brief C++ Library to interface to Arangodb tests.
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
#ifndef DBTEST_H
#define DBTEST_H

#include <gtest/gtest.h>

#include "arangodbcpp/Server.h"
#include "arangodbcpp/Database.h"

class DbTest : public testing::Test {
 public:
  typedef arangodb::dbinterface::Connection Connection;
  typedef arangodb::dbinterface::Database Database;
  typedef arangodb::dbinterface::Server Server;
  DbTest();
  virtual ~DbTest();

 protected:
  void test1();

 private:
  void generalTest(const Connection::VPack (DbTest::*fn)(), uint64_t code);
  void createTest();
  void deleteTest();
  const Connection::VPack createDatabase();
  const Connection::VPack deleteDatabase();

  Server::SPtr _pSrv;
  Database::SPtr _pDb;
  Connection::SPtr _pCon;
};

inline DbTest::~DbTest() {}

#endif  // DBTEST_H
