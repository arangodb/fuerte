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

#include "SrvTest.h"

#include <velocypack/Slice.h>

#include "TestApp.h"

namespace velocypack = arangodb::velocypack;

SrvTest::SrvTest()
    : _pSrv{std::make_shared<Server>(TestApp::hostUrl())},
      _pCon{_pSrv->makeConnection()}

{}

const SrvTest::Connection::VPack SrvTest::getDbVersion() {
  Server& srv = *_pSrv;
  srv.version(_pCon);
  _pCon->run();
  return _pCon->result();
}

TEST_F(SrvTest, version) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  Connection::VPack res = getDbVersion();
  Slice retSlice{res->data()};
  Slice slice = retSlice.get("version");
  if (slice.type() == ValueType::String) {
    auto dbVersion = TestApp::dbVersion();
    if (dbVersion != "*") {
      EXPECT_EQ(dbVersion, TestApp::string(slice));
    }
    return;
  }
  slice = retSlice.get("errorMessage");
  if (slice.type() == ValueType::String) {
    ADD_FAILURE() << TestApp::string(slice);
    return;
  }
  ADD_FAILURE() << "Unexpected failure";
}
