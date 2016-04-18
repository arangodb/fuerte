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
#include "ColTest.h"
#include "TestApp.h"
#include "arangodbcpp/Document.h"

namespace velocypack = arangodb::velocypack;

ColTest::ColTest()
    : _pSrv{std::make_shared<Server>(TestApp::hostUrl(), TestApp::hostPort())},
      _pDb{std::make_shared<Database>(_pSrv, std::string{"Test"})},
      _pCol{std::make_shared<Collection>(_pDb, std::string{"MyTest"})},
      _pCon{std::make_shared<Connection>()} {
  createDatabase();
}

ColTest::~ColTest() { deleteDatabase(); }

const ColTest::Connection::VPack ColTest::createDatabase() {
  _pDb->httpCreate(_pCon);
  _pCon->run();
  return Database::httpCreate(false, _pCon);
}

const ColTest::Connection::VPack ColTest::deleteDatabase() {
  _pDb->httpDelete(_pCon);
  _pCon->run();
  return Database::httpCreate(false, _pCon);
}

const ColTest::Connection::VPack ColTest::createCollection() {
  _pCol->httpCreate(_pCon);
  _pCon->run();
  return Collection::httpCreate(false, _pCon);
}

const ColTest::Connection::VPack ColTest::deleteCollection() {
  _pCol->httpDelete(_pCon);
  _pCon->run();
  return Collection::httpCreate(false, _pCon);
}

const ColTest::Connection::VPack ColTest::truncateCollection() {
  _pCol->httpTruncate(_pCon);
  _pCon->run();
  return Collection::httpTruncate(false, _pCon);
}

const ColTest::Connection::VPack ColTest::docCount() {
  _pCol->httpCount(_pCon);
  _pCon->run();
  return Collection::httpCount(false, _pCon);
}

const ColTest::Connection::VPack ColTest::collections() {
  _pCol->httpCollections(_pCon);
  _pCon->run();
  return Collection::httpCollections(false, _pCon);
}

const ColTest::Connection::VPack ColTest::collectionProps() {
  _pCol->httpProps(_pCon);
  _pCon->run();
  return Collection::httpProps(false, _pCon);
}

const ColTest::Connection::VPack ColTest::aboutCollection() {
  _pCol->httpAbout(_pCon);
  _pCon->run();
  return Collection::httpAbout(false, _pCon);
}

const ColTest::Connection::VPack ColTest::collectionDocs(
    const Collection::Options opts) {
  _pCol->httpDocs(_pCon, opts);
  _pCon->run();
  return Collection::httpDocs(false, _pCon);
}

const ColTest::Connection::VPack ColTest::addDocument(const std::string& name) {
  typedef arangodb::dbinterface::Document Document;
  typedef Document::Options Options;
  Document::SPtr pDoc = std::make_shared<Document>(name);
  pDoc->httpCreate(_pCol, _pCon, Options{"", Options::CreateCol::Yes});
  _pCon->run();
  return Document::httpCreate(false, _pCon);
}

const ColTest::Connection::VPack ColTest::renameCollection(
    const std::string& name) {
  _pCol->httpRename(_pCon, name);
  _pCon->run();
  return Collection::httpRename(false, _pCon);
}

namespace {

void unrecognisedError() { ADD_FAILURE() << std::string{"Unrecognised error"}; }

bool checkKey(const arangodb::velocypack::Slice& resSlice,
              const std::string& name) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  Slice slice = resSlice.get("name");
  if (slice.type() != ValueType::String) {
    return false;
  }
  EXPECT_EQ(name, TestApp::string(slice));
  return true;
}

bool checkError(const arangodb::velocypack::Slice& resSlice) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  Slice slice = resSlice.get("errorMessage");
  if (slice.type() != ValueType::String) {
    return false;
  }
  return true;
}

bool checkResult(const arangodb::velocypack::Slice& resSlice) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  do {
    Slice slice = resSlice.get("error");
    if (slice.type() != ValueType::Bool) {
      unrecognisedError();
      break;
    }
    if (slice.getBool() == false) {
      slice = resSlice.get("code");
      if (slice.type() == ValueType::UInt) {
        EXPECT_EQ(200, slice.getUInt());
        return true;
      }
      unrecognisedError();
    }
  } while (false);
  return false;
}
}

void ColTest::countTest(const uint16_t cnt) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  SCOPED_TRACE("Count");
  const Connection::VPack res = docCount();
  const Slice resSlice = Slice{res->data()};
  if (checkError(resSlice)) {
    return;
  }
  do {
    if (!checkResult(resSlice)) {
      break;
    }
    Slice slice = resSlice.get("count");
    if (slice.type() == ValueType::SmallInt) {
      EXPECT_EQ(cnt, slice.getSmallInt());
      return;
    }
  } while (false);
  unrecognisedError();
}

void ColTest::renameTest(const std::string& name) {
  typedef velocypack::Slice Slice;
  SCOPED_TRACE("Rename");
  const Connection::VPack res = renameCollection(name);
  const Slice resSlice = Slice{res->data()};
  if (checkError(resSlice)) {
    return;
  }
  if (checkResult(resSlice)) {
    if (checkKey(resSlice, name)) {
      return;
    }
  }
  unrecognisedError();
}

void ColTest::docsTest(const std::string& docKey,
                       const Collection::Options opts) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  SCOPED_TRACE("Docs");
  const Connection::VPack res = collectionDocs(opts);
  const Slice resSlice = Slice{res->data()};
  if (checkError(resSlice)) {
    return;
  }
  Slice slice = resSlice.get("documents");
  if (slice.type() != ValueType::Array) {
    unrecognisedError();
    return;
  }
  if (slice.length() != 1) {
    ADD_FAILURE() << "Only 1 Document expected";
    return;
  }
  slice = slice[0];
  const std::string docName = TestApp::string(slice);
  const std::string url = _pCol->refDocUrl(docKey);
  std::string::size_type len = url.length() - docName.length();
  std::string::difference_type cnt =
      std::count(docName.cbegin(), docName.cend(), '/');
  typedef Collection::Options::List List;
  switch (opts.flag<List>()) {
    case List::Path: {
      cnt -= 6;
      break;
    }
    case List::Id: {
      --cnt;
    }
    case List::Key:
    default:;
  }
  if (cnt != 0 || url.substr(len) != docName) {
    ADD_FAILURE() << "Doc list error";
  }
}

void ColTest::commonTest(const Connection::VPack (ColTest::*pTestFnc)(),
                         std::string&& scope) {
  typedef velocypack::Slice Slice;
  SCOPED_TRACE(scope);
  const Connection::VPack res = (this->*pTestFnc)();
  const Slice resSlice = Slice{res->data()};
  if (checkError(resSlice)) {
    return;
  }
  if (checkResult(resSlice)) {
    if (checkKey(resSlice, *_pCol)) {
      return;
    }
  }
  unrecognisedError();
}

void ColTest::deleteTest() {
  typedef velocypack::Slice Slice;
  SCOPED_TRACE("Delete");
  Connection::VPack res = deleteCollection();
  const Slice resSlice = Slice{res->data()};
  if (checkError(resSlice)) {
    return;
  }
  if (!checkResult(resSlice)) {
    unrecognisedError();
  }
}

void ColTest::test1() {
  createTest();
  countTest(0);
  addDocument("newDoc1");
  countTest(1);
  addDocument("newDoc2");
  countTest(2);
  deleteTest();
}

void ColTest::test2() {
  typedef Collection::Options ColOpts;
  typedef ColOpts::List List;
  static std::string newName{"NewTest"};
  addDocument("newDoc1");
  docsTest("newDoc1");
  docsTest("newDoc1", ColOpts{List::Id});
  docsTest("newDoc1", ColOpts{List::Key});
  countTest(1);
  aboutTest();
  renameTest(newName);
  *_pCol = newName;
  docsTest("newDoc1");
  addDocument("newDoc2");
  countTest(2);
  aboutTest();
  truncateTest();
  countTest(0);
  deleteCollection();
}

TEST_F(ColTest, test2) {
  SCOPED_TRACE("test2");
  test2();
}

TEST_F(ColTest, test1) {
  SCOPED_TRACE("test1");
  test1();
}

TEST(test3, colOptsTest) {
  typedef ColTest::Collection::Options Options;
  Options opts{Options::Run::Async, Options::List::Id, Options::Run::Sync,
               Options::List::Key};
  EXPECT_EQ(Options::Run::Sync, opts.flag<Options::Run>());
  EXPECT_EQ(Options::List::Key, opts.flag<Options::List>());
}
