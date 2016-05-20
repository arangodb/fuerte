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

#include "BucketReadTest.h"

#include <thread>

#include "FuerteBench.h"

BucketReadTest::BucketReadTest(const std::string& hostName,
                               const std::string& dbName,
                               const std::string& colName,
                               Connection::Protocol prot)
    : BucketTest(hostName, dbName, colName, prot),
      _document{std::make_shared<Document>("MyDoc")} {}

bool BucketReadTest::collectionExists() {
  enum : long { ReadSuccess = 200 };
  Connection& con = *_connection;
  _collection->about(_connection);
  con.run();
  return con.responseCode() == ReadSuccess;
}

bool BucketReadTest::databaseExists() {
  enum : long { ReadSuccess = 200 };
  Connection& con = *_connection;
  _collection->collections(_connection);
  con.run();
  return con.responseCode() == ReadSuccess;
}

bool BucketReadTest::serverExists() {
  enum : long { ReadSuccess = 200 };
  Connection& con = *_connection;
  _server->version(_connection);
  con.run();
  return con.responseCode() == ReadSuccess;
}

void BucketReadTest::operator()(std::atomic_bool& bWait, LoopCount loops) {
  namespace chrono = std::chrono;
  using system_clock = chrono::system_clock;
  Document& doc = *_document;
  Connection& con = *_connection;

  while (bWait == true) {
    std::this_thread::yield();
  }

  _failed = 0;
  _successful = 0;
  system_clock::time_point now = system_clock::now();

  do {
    DocNames::const_iterator iName = _iFirst;

    do {
      enum : long { ReadSuccess = 200 };
      std::string name = *iName;
      doc = name;
      doc.get(_collection, _connection);
      con.run();
      con.result(false);

      if (con.responseCode() != ReadSuccess) {
        ++_failed;
      } else {
        ++_successful;
      }
    } while (++iName != _iEnd);
  } while (--loops);

  _duration =
      chrono::duration_cast<chrono::microseconds>(system_clock::now() - now);
}
