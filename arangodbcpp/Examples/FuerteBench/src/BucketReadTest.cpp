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
/// Copyright holder is ArangoDB GmbH, Cologne, Ge  `rmany
///
/// @author John Bufton
////////////////////////////////////////////////////////////////////////////////

#include "BucketReadTest.h"

#include <thread>

#include "FuerteBench.h"

BucketReadTest::BucketReadTest(const std::string& hostName,
                               const std::string& dbName,
                               const std::string& colName)
    : _pSrv{std::make_shared<Server>(FuerteBench::hostUrl())},
      _pDb{std::make_shared<Database>(_pSrv, dbName)},
      _pCol{std::make_shared<Collection>(_pDb, colName)},
      _pDoc{std::make_shared<Document>("MyDoc")} {
  if (!hostName.empty()) {
    _pSrv->setHostUrl(hostName);
  }
  _pCon = _pSrv->makeConnection();
}

//
// Ensure the Connection and Document objects are unique
//
// This is required to avoid thread conflicts
//
void BucketReadTest::isolate() {
  _pDoc = std::make_shared<Document>("MyDoc");
  _pCon = _pSrv->makeConnection();
}

bool BucketReadTest::isIsolated() const {
  return _pDoc.unique() && _pCon.unique();
}

bool BucketReadTest::collectionExists() {
  enum : long { ReadSuccess = 200 };
  ConnectionBase& con = *_pCon;
  _pCol->about(_pCon);
  con.run();
  return con.responseCode() == ReadSuccess;
}

bool BucketReadTest::databaseExists() {
  enum : long { ReadSuccess = 200 };
  ConnectionBase& con = *_pCon;
  _pCol->collections(_pCon);
  con.run();
  return con.responseCode() == ReadSuccess;
}

bool BucketReadTest::serverExists() {
  enum : long { ReadSuccess = 200 };
  ConnectionBase& con = *_pCon;
  _pSrv->version(_pCon);
  con.run();
  return con.responseCode() == ReadSuccess;
}

void BucketReadTest::operator()(std::atomic_bool& bWait, LoopCount loops) {
  namespace chrono = std::chrono;
  using system_clock = chrono::system_clock;
  Document& doc = *_pDoc;
  ConnectionBase& con = *_pCon;
  while (bWait == true) {
    std::this_thread::yield();
  }
  _misses = 0;
  _reads = loops * (_iEnd - _iFirst);
  system_clock::time_point now = system_clock::now();
  do {
    DocNames::const_iterator iName = _iFirst;
    do {
      enum : long { ReadSuccess = 200 };
      std::string name = *iName;
      doc = name;
      doc.get(_pCol, _pCon);
      con.run();
      con.result(false);
      if (con.responseCode() != ReadSuccess) {
        ++_misses;
      }
    } while (++iName != _iEnd);
  } while (--loops);
  _usecs =
      chrono::duration_cast<chrono::microseconds>(system_clock::now() - now);
}
