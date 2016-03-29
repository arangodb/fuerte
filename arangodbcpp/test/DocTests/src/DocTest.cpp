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

//#include <sstream>

#include "DocTest.h"
#include "TestApp.h"

namespace velocypack = arangodb::velocypack;

namespace {

inline void httpRun(DocTest::Connection& con) {
  do {
    con.run();
  } while (con.isRunning());
}
}

const DocTest::Connection::VPack DocTest::makeDocument() {
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
    builder("_key", Value(key));
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

DocTest::DocTest()
    : _pSrv{std::make_shared<Server>(TestApp::hostUrl(), TestApp::hostPort())},
      _pDb{std::make_shared<Database>(_pSrv, std::string{"Test"})},
      _pCol{std::make_shared<Collection>(_pDb, std::string{"MyTest"})},
      _pDoc{std::make_shared<Document>("MyDoc")},
      _pCon{std::make_shared<Connection>()} {
  createDatabase();
  createCollection();
}

DocTest::~DocTest() { deleteDatabase(); }

//-------- Document interfaces ----------------------

const DocTest::Connection::VPack DocTest::docHead(const Document::Options& opts,
                                                  bool bSort) {
  _pDoc->httpHead(_pCol, _pCon, opts);
  httpRun(*_pCon);
  return Document::httpHead(bSort, _pCon);
}

const DocTest::Connection::VPack DocTest::addDocument(
    const Connection::VPack& doc, const Document::Options& opts) {
  _pDoc->httpCreate(_pCol, _pCon, doc, opts);
  httpRun(*_pCon);
  return Document::httpCreate(false, _pCon);
}

const DocTest::Connection::VPack DocTest::deleteDoc(
    const Document::Options& opts) {
  _pDoc->httpDelete(_pCol, _pCon, opts);
  httpRun(*_pCon);
  return Document::httpDelete(false, _pCon);
}

//---------------------------------------------------

//-------- Database interfaces ----------------------

const DocTest::Connection::VPack DocTest::createDatabase() {
  _pDb->httpCreate(_pCon);
  httpRun(*_pCon);
  return Database::httpCreate(false, _pCon);
}

const DocTest::Connection::VPack DocTest::deleteDatabase() {
  Database& db = *_pDb;
  db.httpDelete(_pCon);
  httpRun(*_pCon);
  return Database::httpCreate(false, _pCon);
}

//---------------------------------------------------

//-------- Collection interfaces ----------------------

const DocTest::Connection::VPack DocTest::createCollection() {
  _pCol->httpCreate(_pCon);
  httpRun(*_pCon);
  return Collection::httpCreate(false, _pCon);
}

const DocTest::Connection::VPack DocTest::deleteCollection() {
  _pCol->httpDelete(_pCon);
  httpRun(*_pCon);
  return Collection::httpCreate(false, _pCon);
}

const DocTest::Connection::VPack DocTest::truncateCollection() {
  _pCol->httpTruncate(_pCon);
  httpRun(*_pCon);
  return Collection::httpTruncate(false, _pCon);
}

//-----------------------------------------------------

namespace {

void unrecognisedError() { ADD_FAILURE() << std::string{"Unrecognised error"}; }

#if 0

bool checkKey(const arangodb::velocypack::Slice &resSlice, const std::string &name)
{
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  Slice slice = resSlice.get("_key");
  if (slice.type() != ValueType::String)
  {
    return false;
  }
  EXPECT_EQ(name,TestApp::string(slice));
  return true;
}

#endif

bool checkError(const arangodb::velocypack::Slice& resSlice) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  Slice slice = resSlice.get("error");
  if (slice.type() != ValueType::Bool) {
    unrecognisedError();
    return true;
  }
  if (slice.getBool() == true) {
    slice = resSlice.get("errorMessage");
    if (slice.type() != ValueType::String) {
      unrecognisedError();
      return false;
    }
    std::ostringstream msg{TestApp::string(slice)};
    msg << std::endl;
    slice = resSlice.get("code");
    if (slice.type() != ValueType::UInt) {
      unrecognisedError();
      return false;
    }
    msg << "Return code : " << slice.getUInt();
    ADD_FAILURE() << msg.str();
    return true;
  }
  return false;
}
}

void DocTest::renameTest(const std::string&  // name
                         ) {}

bool DocTest::checkKey(const arangodb::velocypack::Slice& resSlice) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  Slice slice = resSlice.get("_key");
  if (slice.type() != ValueType::String) {
    unrecognisedError();
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
  unsigned res = _pCon->httpResponseCode();
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

std::string DocTest::headTest(const Document::Options& opts) {
  typedef velocypack::Slice Slice;
  typedef velocypack::ValueType ValueType;
  const Connection::VPack res = docHead(opts);
  Slice resSlice{res->data()};
  Slice slice = resSlice.get("Server");
  SCOPED_TRACE("Test");
  std::string ret;
  do {
    if (slice.type() != ValueType::String) {
      unrecognisedError();
      break;
    }
    EXPECT_EQ("ArangoDB", ret);
    slice = resSlice.get("Etag");
    if (slice.type() != ValueType::String) {
      unrecognisedError();
      break;
    }
    ret = TestApp::string(slice);
  } while (false);
  return ret;
}

void DocTest::deleteTest(const Document::Options& opts) {
  typedef Document::Options::Sync Sync;
  typedef velocypack::Slice Slice;
  const Connection::VPack res = deleteDoc(opts);
  Slice resSlice{res->data()};
  SCOPED_TRACE("deleteTest");
  enum : uint16_t { WaitSuccess = 200, NoWaitSuccess = 202, RevResponse = 412 };
  do {
    if (_pCon->httpResponseCode() == RevResponse) {
      checkKey(resSlice);
      break;
    }
    if (checkError(resSlice)) {
      break;
    }
    if (checkKey(resSlice)) {
      checkResponse(WaitSuccess, NoWaitSuccess, opts.flag<Sync>());
    }
  } while (false);
}

void DocTest::createTest(const Connection::VPack& doc,
                         const Document::Options& opts) {
  typedef Document::Options::Sync Sync;
  typedef velocypack::Slice Slice;
  const Connection::VPack res = addDocument(doc, opts);
  Slice resSlice{res->data()};
  SCOPED_TRACE("createTest");
  enum : uint16_t { WaitSuccess = 201, NoWaitSuccess = 202 };
  do {
    if (checkError(resSlice)) {
      break;
    }

    if (checkKey(resSlice)) {
      checkResponse(WaitSuccess, NoWaitSuccess, opts.flag<Sync>());
    }
  } while (false);
}

void DocTest::test1() {
  typedef Document::Options Options;
  typedef Options::Sync Sync;
  typedef Options::Run Run;
  typedef Options::CreateCol CreateCol;
  typedef Connection::VPack VPack;
  SCOPED_TRACE("test1");
  VPack doc1 = makeDocument();
  Options opts{"12345678", Sync::Wait, Run::Sync};
  createTest(doc1, opts);
  deleteTest(opts);
  deleteCollection();
  opts.setFlags(Sync::NoWait, Run::Async, CreateCol::Yes);
  createTest(doc1, opts);
  deleteTest(opts);
}

void DocTest::test2() { SCOPED_TRACE("test2"); }

TEST_F(DocTest, test2) { test2(); }

TEST_F(DocTest, test1) { test1(); }

TEST(DocOptsTest, test3) {
  typedef DocTest::Document::Options Options;
  typedef Options::Rev Rev;
  typedef Options::Merge Merge;
  typedef Options::CreateCol CreateCol;
  typedef Options::CreateColType CreateColType;
  typedef Options::Run Run;
  typedef Options::Merge Merge;
  typedef Options::Policy Policy;
  Options opts{"1234",         Rev::Match,          Rev::NoMatch,
               Run::Async,     Run::Sync,           Merge::No,
               Merge::Yes,     CreateColType::Edge, CreateColType::Document,
               CreateCol::Yes, CreateCol::No,       Policy::Error,
               Policy::Last};
  EXPECT_EQ(Rev::NoMatch, opts.flag<Rev>());
  EXPECT_EQ(Run::Sync, opts.flag<Run>());
  EXPECT_EQ(Merge::Yes, opts.flag<Merge>());
  EXPECT_EQ(CreateColType::Document, opts.flag<CreateColType>());
  EXPECT_EQ(CreateCol::No, opts.flag<CreateCol>());
  EXPECT_EQ(Policy::Last, opts.flag<Policy>());
}
