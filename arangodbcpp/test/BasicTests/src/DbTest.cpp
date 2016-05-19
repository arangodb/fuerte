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

#include "DbTest.h"

#include "TestApp.h"

namespace velocypack = arangodb::velocypack;

DbTest::DbTest()
    : _pSrv{std::make_shared<Server>(TestApp::hostUrl())},
      _pDb{std::make_shared<Database>(_pSrv, std::string{"Test"})},
      _pCon{_pSrv->makeConnection()}

{}

const DbTest::ConnectionBase::VPack DbTest::createDatabase() {
  Database& db = *_pDb;
  db.create(_pCon);
  _pCon->run();
  return _pCon->result(false);
}

const DbTest::ConnectionBase::VPack DbTest::deleteDatabase() {
  Database& db = *_pDb;
  db.remove(_pCon);
  _pCon->run();
  return _pCon->result(false);
}

void DbTest::generalTest(const ConnectionBase::VPack (DbTest::*fn)(),
                         uint64_t code) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  const ConnectionBase::VPack res = (this->*fn)();
  Slice retSlice{res->data()};
  Slice slice = retSlice.get("result");
  if (slice.type() == ValueType::Bool) {
    EXPECT_EQ(true, slice.getBool());
  }
  slice = retSlice.get("code");
  if (slice.type() == ValueType::UInt) {
    EXPECT_EQ(code, slice.getUInt());
    return;
  }
  slice = retSlice.get("errorMessage");
  if (slice.type() == ValueType::String) {
    ADD_FAILURE() << TestApp::string(slice);
    return;
  }
  ADD_FAILURE() << "Unexpected failure";
}

void DbTest::createTest() {
  SCOPED_TRACE("Create");
  generalTest(&DbTest::createDatabase, 201);
}

void DbTest::deleteTest() {
  SCOPED_TRACE("Delete");
  generalTest(&DbTest::deleteDatabase, 200);
}

void DbTest::test1() {
  createTest();
  deleteTest();
}

TEST_F(DbTest, test1) {
  SCOPED_TRACE("test1");
  test1();
}
