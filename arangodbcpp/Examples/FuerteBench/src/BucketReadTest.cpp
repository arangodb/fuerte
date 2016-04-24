////////////////////////////////////////////////////////////////////////////////
/// @brief C++ Library to interface to Arangodb.
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
#include <thread>

#include "BucketReadTest.h"
#include "FuerteBench.h"

BucketReadTest::BucketReadTest(const std::string& hostName,
                               const std::string& dbName,
                               const std::string& colName)
    : _pSrv{std::make_shared<Server>(FuerteBench::hostUrl(),
                                     FuerteBench::hostPort())},
      _pDb{std::make_shared<Database>(_pSrv, dbName)},
      _pCol{std::make_shared<Collection>(_pDb, colName)},
      _pDoc{std::make_shared<Document>("MyDoc")},
      _pCon{std::make_shared<Connection>()} {
  if (!hostName.empty()) {
    _pSrv->setHostUrl(hostName);
  }
}

void BucketReadTest::isolate() {
  _pDoc = std::make_shared<Document>("MyDoc");
  _pCon = std::make_shared<Connection>();
}

bool BucketReadTest::isIsolated() const {
  return _pDoc.unique() && _pCon.unique();
}

bool BucketReadTest::collectionExists() {
  enum : long { ReadSuccess = 200 };
  Connection& con = *_pCon;
  _pCol->httpAbout(_pCon);
  con.run();
  return con.httpResponseCode() == ReadSuccess;
}

bool BucketReadTest::databaseExists() {
  enum : long { ReadSuccess = 200 };
  Connection& con = *_pCon;
  _pCol->httpCollections(_pCon);
  con.run();
  return con.httpResponseCode() == ReadSuccess;
}

bool BucketReadTest::serverExists() {
  enum : long { ReadSuccess = 200 };
  Connection& con = *_pCon;
  _pSrv->httpVersion(_pCon);
  con.run();
  std::cout << "httpEffectiveUrl " << con.httpEffectiveUrl() << "\n";
  std::cout << "bufString " << con.bufString() << "\n";
  std::cout << "http response code " << con.httpResponseCode() << "\n";
  return con.httpResponseCode() == ReadSuccess;
}

void BucketReadTest::operator()(std::atomic_bool& bWait, LoopCount loops) {
  namespace chrono = std::chrono;
  using system_clock = chrono::system_clock;
  Document& doc = *_pDoc;
  Connection& con = *_pCon;
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
      doc.httpGet(_pCol, _pCon);
      con.run();
      Document::httpGet(false, _pCon);
      if (con.httpResponseCode() != ReadSuccess) {
        ++_misses;
      }
    } while (++iName != _iEnd);
  } while (--loops);
  _usecs =
      chrono::duration_cast<chrono::microseconds>(system_clock::now() - now);
}
