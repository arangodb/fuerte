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

#include "DocTest.h"

#include "TestApp.h"

namespace velocypack = arangodb::velocypack;

namespace {

inline void httpRun(DocTest::ConnectionBase& con, bool bAsync = false) {
  con.setAsynchronous(bAsync);
  do {
    con.run();
  } while (con.isRunning());
}

namespace attrib {
const std::string code{"code"};
const std::string error{"error"};
const std::string errMsg{"errorMessage"};
const std::string key{"_key"};
const std::string rev{"_rev"};
const std::string server{"Server"};
}
}

const DocTest::ConnectionBase::VPack DocTest::makeDocument1() {
  using velocypack::ValueType;
  using velocypack::Value;
  using velocypack::Options;
  using velocypack::Builder;
  using velocypack::ValuePair;
  using std::string;
  Builder builder;
  // Start building an object
  builder(Value(ValueType::Object));
  {
    const std::string& key = *_pDoc;
    builder(attrib::key, Value(key));
  }
  builder("House No", Value(12))("Male", Value(true));
  // Build an array
  builder("Stuff", Value(ValueType::Array))(Value(1))(Value("Hello"))(Value(2))(
      Value("World"))(Value(3))
      .close();
  // Finish the object
  builder("Name", Value("Gustav")).close();
  return builder.steal();
}

const DocTest::ConnectionBase::VPack DocTest::makeDocument2() {
  using velocypack::ValueType;
  using velocypack::Value;
  using velocypack::Options;
  using velocypack::Builder;
  using velocypack::ValuePair;
  using std::string;
  Builder builder;
  // Start building an object
  builder(Value(ValueType::Object));
  {
    const std::string& key = *_pDoc;
    builder(attrib::key, Value(key));
  }
  builder("Planet No", Value(3))("Name", Value("Blue"));
  // Build an array
  builder("Planets", Value(ValueType::Array))(Value(3))(Value("Mercury"))(
      Value("Venus"))(Value("Earth"))(Value(2))
      .close();
  // Finish the object
  builder("Star", Value("Sun")).close();
  return builder.steal();
}

DocTest::DocTest()
    : _pSrv{std::make_shared<Server>(TestApp::hostUrl())},
      _pDb{std::make_shared<Database>(_pSrv, std::string{"Test"})},
      _pCol{std::make_shared<Collection>(_pDb, std::string{"MyTest"})},
      _pDoc{std::make_shared<Document>("MyDoc")} {
  _pCon = _pSrv->makeConnection();
  createDatabase();
  createCollection();
}

DocTest::~DocTest() { deleteDatabase(); }

//-------- Document interfaces ----------------------

const DocTest::ConnectionBase::VPack DocTest::getDoc(
    const Document::Options& opts) {
  _pDoc->get(_pCol, _pCon, opts);
  httpRun(*_pCon);
  return _pCon->result(false);
}

const DocTest::ConnectionBase::VPack DocTest::replaceDoc(
    const ConnectionBase::VPack& doc, const Document::Options& opts) {
  _pDoc->replace(_pCol, _pCon, doc, opts);
  httpRun(*_pCon);
  return _pCon->result(false);
}

const DocTest::ConnectionBase::VPack DocTest::createDoc(
    const ConnectionBase::VPack& doc, const Document::Options& opts) {
  _pDoc->create(_pCol, _pCon, doc, opts);
  httpRun(*_pCon);
  return _pCon->result(false);
}

const DocTest::ConnectionBase::VPack DocTest::patchDoc(
    const ConnectionBase::VPack& doc, const Document::Options& opts) {
  _pDoc->patch(_pCol, _pCon, doc, opts);
  httpRun(*_pCon);
  return _pCon->result(false);
}

const DocTest::ConnectionBase::VPack DocTest::createDoc(
    const Document::Options& opts) {
  _pDoc->create(_pCol, _pCon, opts);
  httpRun(*_pCon);
  return _pCon->result(false);
}

const DocTest::ConnectionBase::VPack DocTest::deleteDoc(
    const Document::Options& opts) {
  _pDoc->remove(_pCol, _pCon, opts);
  httpRun(*_pCon);
  return _pCon->result(false);
}

//---------------------------------------------------

//-------- Database interfaces ----------------------

const DocTest::ConnectionBase::VPack DocTest::createDatabase() {
  _pDb->create(_pCon);
  httpRun(*_pCon);
  return _pCon->result(false);
}

const DocTest::ConnectionBase::VPack DocTest::deleteDatabase() {
  _pDb->remove(_pCon);
  httpRun(*_pCon);
  return _pCon->result(false);
}

//---------------------------------------------------

//-------- Collection interfaces ----------------------

const DocTest::ConnectionBase::VPack DocTest::createCollection() {
  _pCol->create(_pCon);
  httpRun(*_pCon);
  return _pCon->result(false);
}

const DocTest::ConnectionBase::VPack DocTest::deleteCollection() {
  _pCol->remove(_pCon);
  httpRun(*_pCon);
  return _pCon->result(false);
}

const DocTest::ConnectionBase::VPack DocTest::truncateCollection() {
  _pCol->truncate(_pCon);
  httpRun(*_pCon);
  return _pCon->result(false);
}

//-----------------------------------------------------

//-------- Server interfaces ----------------------

const DocTest::ConnectionBase::VPack DocTest::serverVer() {
  _pSrv->version(_pCon);
  httpRun(*_pCon);
  return _pCon->result(false);
}

//-------------------------------------------------

namespace {

void attribNotFound(const std::string& attrib) {
  ADD_FAILURE() << attrib << " attribute not found";
}
}

bool DocTest::checkError(const velocypack::Slice& resSlice) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  Slice slice = resSlice.get(attrib::error);
  if (slice.type() != ValueType::Bool) {
    attribNotFound(attrib::error);
    return true;
  }
  if (slice.getBool() == false) {
    return false;
  }
  do {
    slice = resSlice.get(attrib::errMsg);
    if (slice.type() != ValueType::String) {
      attribNotFound(attrib::errMsg);
      break;
    }
    slice = resSlice.get(attrib::code);
    if (slice.type() != ValueType::UInt) {
      attribNotFound(attrib::code);
      break;
    }
    EXPECT_EQ((unsigned int)_pCon->responseCode(), slice.getUInt());
  } while (false);
  return true;
}

bool DocTest::checkKey(const arangodb::velocypack::Slice& resSlice) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  Slice slice = resSlice.get(attrib::key);
  if (slice.type() != ValueType::String) {
    attribNotFound(attrib::key);
    return false;
  }
  {
    const std::string& key = *_pDoc;
    EXPECT_EQ(key, TestApp::string(slice));
  }
  return true;
}

void DocTest::checkResponse(const unsigned rWait, unsigned rNoWait,
                            Document::Options::Sync flg) {
  typedef Document::Options::Sync Sync;
  unsigned res = _pCon->responseCode();
  switch (flg) {
    case Sync::NoWait: {
      EXPECT_EQ(rNoWait, res);
      break;
    }
    case Sync::Wait: {
      EXPECT_EQ(rWait, res);
      break;
    }
    default:;
  }
}

bool DocTest::versionTest() {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  const ConnectionBase::VPack res = serverVer();
  Slice resSlice{res->data()};
  Slice slice = resSlice.get(attrib::errMsg);
  if (slice.type() == ValueType::String) {
    ADD_FAILURE() << TestApp::string(slice);
    return false;
  }
  return true;
}

std::string DocTest::getRev() {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  typedef Document::Options Options;
  const ConnectionBase::VPack res = getDoc(Options());
  Slice resSlice{res->data()};
  Slice slice = resSlice.get(attrib::rev);
  SCOPED_TRACE("getRev");
  if (slice.type() != ValueType::String) {
    attribNotFound(attrib::rev);
    return std::string();
  }
  return TestApp::string(slice);
}

bool DocTest::getTest(const Document::Options& opts) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  const ConnectionBase::VPack res = getDoc(opts);
  Slice resSlice{res->data()};
  SCOPED_TRACE("getTest");
  enum : uint16_t { RevMatch = 412, DocNotFound = 404, RevNoMatch = 304 };
  switch (_pCon->responseCode()) {
    case RevNoMatch: {
      Slice slice = resSlice.get(attrib::code);
      if (slice.type() != ValueType::UInt) {
        attribNotFound(attrib::code);
        break;
      }
      EXPECT_EQ(RevNoMatch, slice.getUInt());
      slice = resSlice.get(attrib::error);
      if (slice.type() != ValueType::Bool) {
        attribNotFound(attrib::error);
        break;
      }
      EXPECT_NE(true, slice.getBool());
      break;
    }
    case RevMatch: {
      checkKey(resSlice);
      // Deliberately drop through
    }
    case DocNotFound: {
      checkError(resSlice);
      break;
    }
    default: {
      checkKey(resSlice);
      return true;
    }
  }
  return false;
}

bool DocTest::deleteTest(const Document::Options& opts) {
  typedef velocypack::Slice Slice;
  const ConnectionBase::VPack res = deleteDoc(opts);
  Slice resSlice{res->data()};
  SCOPED_TRACE("deleteTest");
  enum : uint16_t { WaitSuccess = 200, NoWaitSuccess = 202, RevMatch = 412 };
  switch (_pCon->responseCode()) {
    case WaitSuccess:
    case NoWaitSuccess: {
      return checkKey(resSlice);
    }

    default: { checkError(resSlice); }
  }
  return false;
}

bool DocTest::replaceTest(const ConnectionBase::VPack& doc,
                          const Document::Options& opts) {
  typedef velocypack::Slice Slice;
  const ConnectionBase::VPack res = replaceDoc(doc, opts);
  Slice resSlice{res->data()};
  unsigned code = _pCon->responseCode();
  SCOPED_TRACE("replaceTest");
  enum : uint16_t {
    WaitSuccess = 201,
    NoWaitSuccess = 202,
    NotFound = 404,
    NotMatched = 412
  };
  switch (code) {
    case WaitSuccess:
    case NoWaitSuccess: {
      const std::string key = *_pDoc;
      Slice slice = resSlice.get(attrib::key);
      EXPECT_EQ(key, TestApp::string(slice));
      return true;
    }

    case NotMatched:
    case NotFound: {
      Slice slice = resSlice.get(attrib::code);
      EXPECT_EQ(code, slice.getUInt());
    }
    default:;
  }
  return false;
}

bool DocTest::createTest(const ConnectionBase::VPack& doc,
                         const Document::Options& opts) {
  typedef velocypack::Slice Slice;
  const ConnectionBase::VPack res = createDoc(doc, opts);
  Slice resSlice{res->data()};
  unsigned code = _pCon->responseCode();
  SCOPED_TRACE("createTest");
  enum : uint16_t {
    WaitSuccess = 201,
    NoWaitSuccess = 202,
    Invalid = 400,
    NoCollection = 404
  };
  switch (code) {
    case WaitSuccess:
    case NoWaitSuccess: {
      const std::string key = *_pDoc;
      Slice slice = resSlice.get(attrib::key);
      EXPECT_EQ(key, TestApp::string(slice));
      return true;
    }

    case Invalid:
    case NoCollection: {
      Slice slice = resSlice.get(attrib::code);
      EXPECT_EQ(code, slice.getUInt());
    }
    default:;
  }
  return false;
}

bool DocTest::patchTest(const ConnectionBase::VPack& doc,
                        const Document::Options& opts) {
  typedef Document::Options::Sync Sync;
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  const ConnectionBase::VPack res = patchDoc(doc, opts);
  Slice resSlice{res->data()};
  SCOPED_TRACE("patchTest");
  enum : uint16_t { WaitSuccess = 201, NoWaitSuccess = 202 };
  do {
    Slice slice = resSlice.get(attrib::key);
    if (slice.type() == ValueType::String) {
      *_pDoc = TestApp::string(slice);
      checkResponse(WaitSuccess, NoWaitSuccess, opts.flag<Sync>());
      break;
    }
    checkError(resSlice);
    return false;
  } while (false);
  return true;
}

void DocTest::test1() {
  typedef Document::Options Options;
  typedef Options::Sync Sync;
  typedef ConnectionBase::VPack VPack;
  SCOPED_TRACE("test1");
  VPack docData = makeDocument1();
  Options opts{"1234", Sync::Wait};
  EXPECT_EQ(true, createTest(docData, opts));
  EXPECT_EQ(true, deleteTest(opts));
  truncateCollection();
}

void DocTest::test2() {
  typedef Document::Options Options;
  typedef Options::Rev Rev;
  typedef Options::Sync Sync;
  typedef ConnectionBase::VPack VPack;
  SCOPED_TRACE("test2");
  VPack doc1 = makeDocument1();
  Options opts{"1234", Rev::Match};
  createDoc(doc1, opts);
  EXPECT_NE(true, getTest(opts));
  opts.setFlags(Rev::NoMatch, Sync::Wait);
  EXPECT_EQ(true, getTest(opts));
  opts = getRev();
  EXPECT_NE(true, getTest(opts));
  opts.setFlags(Rev::Match, Sync::NoWait);
  EXPECT_EQ(true, getTest(opts));
  EXPECT_EQ(true, deleteTest(opts));  // Should pass as rev matches
}

void DocTest::test3() {
  typedef Document::Options Options;
  typedef Options::Rev Rev;
  typedef Options::Sync Sync;
  typedef ConnectionBase::VPack VPack;
  SCOPED_TRACE("test3");
  Document& doc = *_pDoc;
  VPack doc1 = makeDocument1();
  doc = std::string{"otherDoc"};
  VPack doc2 = makeDocument2();
  Options opts{"1234"};
  createDoc(opts);  // Creates an empty document
  opts.setFlags(Sync::Wait);
  EXPECT_EQ(true, replaceTest(doc1, opts));
  opts.setFlags(Sync::NoWait);
  EXPECT_EQ(true, replaceTest(doc2, opts));
  opts.setFlags(Rev::Match);
  EXPECT_NE(true, replaceTest(doc1, opts));
  opts = getRev();
  opts.setFlags(Rev::Match);
  EXPECT_EQ(true, replaceTest(doc1, opts));
  truncateCollection();
}

void DocTest::test4() {
  typedef DocTest::Document::Options Options;
  typedef Options::Rev Rev;
  typedef Options::Merge Merge;
  typedef Options::Merge Merge;
  SCOPED_TRACE("test4");
  Options opts{"1234", Rev::Match, Rev::NoMatch, Merge::No, Merge::Yes};
  EXPECT_EQ(Rev::NoMatch, opts.flag<Rev>());
  EXPECT_EQ(Merge::Yes, opts.flag<Merge>());
}

void DocTest::test5() {
  typedef Document::Options Options;
  typedef Options::Sync Sync;
  typedef Options::Merge Merge;
  typedef ConnectionBase::VPack VPack;
  SCOPED_TRACE("test5");
  Document& doc = *_pDoc;
  doc = "otherDoc";
  VPack doc2 = makeDocument2();
  doc = "NewDoc";
  VPack doc1 = makeDocument1();
  Options opts{"1234", Sync::Wait};
  EXPECT_EQ(true, createTest(doc1, opts));
  getDoc(opts);
  EXPECT_EQ(true, patchTest(doc2, opts));
  getDoc(opts);
  opts.setFlags(Sync::NoWait, Merge::No);
  doc = "otherDoc";
  EXPECT_EQ(true, createTest(doc2, opts));
  getDoc(opts);
  EXPECT_EQ(true, patchTest(doc1, opts));
  getDoc(opts);
  truncateCollection();
}

TEST_F(DocTest, DocTests) {
  if (versionTest()) {
    test1();
    test2();
    test3();
    test4();
    test5();
  }
}
