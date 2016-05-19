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

#ifndef DOCTEST_H
#define DOCTEST_H

#include <gtest/gtest.h>

#include "arangodbcpp/Server.h"
#include "arangodbcpp/Database.h"
#include "arangodbcpp/Collection.h"
#include "arangodbcpp/Document.h"

class DocTest : public testing::Test {
 public:
  typedef arangodb::dbinterface::ConnectionBase ConnectionBase;
  typedef arangodb::dbinterface::Database Database;
  typedef arangodb::dbinterface::Server Server;
  typedef arangodb::dbinterface::Collection Collection;
  typedef arangodb::dbinterface::Document Document;
  DocTest();
  virtual ~DocTest();

 protected:
  void test1();
  void test2();
  void test3();
  static void test4();
  void test5();
  bool versionTest();

 private:
  const ConnectionBase::VPack makeDocument1();
  const ConnectionBase::VPack makeDocument2();
  void generalTest(const ConnectionBase::VPack (DocTest::*fn)(), uint64_t code);
  void checkResponse(const unsigned rWait, unsigned rNoWait,
                     Document::Options::Sync flg);
  bool checkKey(const arangodb::velocypack::Slice& resSlice);
  bool createTest(const ConnectionBase::VPack& doc,
                  const Document::Options& opts);
  bool patchTest(const ConnectionBase::VPack& doc,
                 const Document::Options& opts);
  bool replaceTest(const ConnectionBase::VPack& doc,
                   const Document::Options& opts);
  bool getTest(const Document::Options& opts);
  bool deleteTest(const Document::Options& opts);
  void replaceTest(const std::string& name);
  std::string getRev();
  bool checkError(const arangodb::velocypack::Slice& resSlice);
  const ConnectionBase::VPack createDatabase();
  const ConnectionBase::VPack deleteDatabase();
  const ConnectionBase::VPack createCollection();
  const ConnectionBase::VPack deleteCollection();
  const ConnectionBase::VPack truncateCollection();
  const ConnectionBase::VPack serverVer();
  const ConnectionBase::VPack createDoc(
      const ConnectionBase::VPack& doc,
      const Document::Options& opts = Document::Options{});
  const ConnectionBase::VPack patchDoc(
      const ConnectionBase::VPack& doc,
      const Document::Options& opts = Document::Options{});
  const ConnectionBase::VPack createDoc(
      const Document::Options& opts = Document::Options{});
  const ConnectionBase::VPack replaceDoc(
      const ConnectionBase::VPack& doc,
      const Document::Options& opts = Document::Options{});
  const ConnectionBase::VPack deleteDoc(const Document::Options& opts);
  const ConnectionBase::VPack getDoc(const Document::Options& opts);

  Server::SPtr _pSrv;
  Database::SPtr _pDb;
  Collection::SPtr _pCol;
  Document::SPtr _pDoc;
  ConnectionBase::SPtr _pCon;
};

#endif  // DOCTEST_H
