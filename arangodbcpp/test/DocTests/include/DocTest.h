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
#ifndef DOCTEST_H
#define DOCTEST_H

#include <gtest/gtest.h>

#include "arangodbcpp/Server.h"
#include "arangodbcpp/Database.h"
#include "arangodbcpp/Collection.h"
#include "arangodbcpp/Document.h"
#include "arangodbcpp/Connection.h"

class DocTest : public testing::Test {
 public:
  typedef arangodb::dbinterface::Connection Connection;
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
  const DocTest::Connection::VPack makeDocument1();
  const DocTest::Connection::VPack makeDocument2();
  void generalTest(const Connection::VPack (DocTest::*fn)(), uint64_t code);
  void checkResponse(const unsigned rWait, unsigned rNoWait,
                     Document::Options::Sync flg);
  bool checkKey(const arangodb::velocypack::Slice& resSlice);
  bool createTest(const Connection::VPack& doc, const Document::Options& opts);
  bool patchTest(const Connection::VPack& doc, const Document::Options& opts);
  bool replaceTest(const Connection::VPack& doc, const Document::Options& opts);
  bool getTest(const Document::Options& opts);
  bool deleteTest(const Document::Options& opts);
  void replaceTest(const std::string& name);
  std::string headTest(const Document::Options& opts);
  bool checkError(const arangodb::velocypack::Slice& resSlice);
  const Connection::VPack createDatabase();
  const Connection::VPack deleteDatabase();
  const Connection::VPack createCollection();
  const Connection::VPack deleteCollection();
  const Connection::VPack truncateCollection();
  const Connection::VPack serverVer();
  const Connection::VPack createDoc(
      const Connection::VPack& doc,
      const Document::Options& opts = Document::Options{});
  const Connection::VPack patchDoc(
      const Connection::VPack& doc,
      const Document::Options& opts = Document::Options{});
  const Connection::VPack createDoc(
      const Document::Options& opts = Document::Options{});
  const Connection::VPack replaceDoc(
      const Connection::VPack& doc,
      const Document::Options& opts = Document::Options{});
  const Connection::VPack docHead(const Document::Options& opts,
                                  bool bSort = false);
  const Connection::VPack deleteDoc(const Document::Options& opts);
  const Connection::VPack getDoc(const Document::Options& opts);

  Server::SPtr _pSrv;
  Database::SPtr _pDb;
  Collection::SPtr _pCol;
  Document::SPtr _pDoc;
  Connection::SPtr _pCon;
};

#endif  // DOCTEST_H