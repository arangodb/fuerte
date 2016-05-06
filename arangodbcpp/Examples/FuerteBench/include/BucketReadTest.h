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

#ifndef BUCKETREADTEST_H
#define BUCKETREADTEST_H

#include <chrono>
#include <atomic>

#include <arangodbcpp/Server.h>
#include <arangodbcpp/Collection.h>
#include <arangodbcpp/Document.h>

class BucketReadTest {
  typedef arangodb::dbinterface::Server Server;
  typedef arangodb::dbinterface::Database Database;
  typedef arangodb::dbinterface::Connection Connection;
  typedef arangodb::dbinterface::Collection Collection;
  typedef arangodb::dbinterface::Document Document;
  typedef Connection::Protocol Protocol;

 public:
  typedef std::vector<std::string> DocNames;
  typedef uint32_t ReadCount;
  typedef uint16_t LoopCount;
  BucketReadTest() = delete;
  BucketReadTest(const std::string& hostName, const std::string& dbName,
                 const std::string& colName);
  DocNames::const_iterator setDocs(DocNames::const_iterator iFirst,
                                   DocNames::size_type n,
                                   Connection::Protocol prot);
  bool collectionExists();
  bool databaseExists();
  bool serverExists();
  void operator()(std::atomic_bool& bWait, LoopCount loops);
  void isolate();
  bool isIsolated() const;
  std::chrono::microseconds duration() const;
  ReadCount misses() const;
  ReadCount reads() const;
  DocNames::difference_type noDocs() const;

 private:
  Connection::VPack (*_docGet)(const bool, const Connection::SPtr&);
  Server::SPtr _pSrv;
  Database::SPtr _pDb;
  Collection::SPtr _pCol;
  Document::SPtr _pDoc;
  Connection::SPtr _pCon;
  DocNames::const_iterator _iFirst;
  DocNames::const_iterator _iEnd;
  std::chrono::microseconds _usecs;
  ReadCount _misses;
  ReadCount _reads;
  Protocol _protocol;
};

inline BucketReadTest::DocNames::difference_type BucketReadTest::noDocs()
    const {
  return _iEnd - _iFirst;
}

inline BucketReadTest::ReadCount BucketReadTest::misses() const {
  return _misses;
}

inline BucketReadTest::ReadCount BucketReadTest::reads() const {
  return _reads;
}

inline std::chrono::microseconds BucketReadTest::duration() const {
  return _usecs;
}

inline BucketReadTest::DocNames::const_iterator BucketReadTest::setDocs(
    DocNames::const_iterator iFirst, DocNames::size_type n,
    Connection::Protocol prot) {
  _docGet = Document::httpGet;
  _iFirst = iFirst;
  _iEnd = iFirst + n;
  *_pCon = prot;
  if (prot == Connection::Protocol::VPackJSon) {
    _docGet = Document::vppGet;
  }
  return _iEnd;
}

#endif  // BUCKETREADTEST_H
