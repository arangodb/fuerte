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

#include "BucketWriteTest.h"

#include <thread>

#include "FuerteBench.h"

BucketWriteTest::BucketWriteTest(const std::string& hostName,
                                 const std::string& dbName,
                                 const std::string& colName,
                                 ConnectionBase::Protocol prot)
    : BucketTest(hostName, dbName, colName, prot),
      _document{std::make_shared<Document>()} {}

void BucketWriteTest::operator()(std::atomic_bool& bWait, LoopCount loops) {
  namespace chrono = std::chrono;
  using system_clock = chrono::system_clock;
  Document& doc = *_document;
  ConnectionBase& con = *_connection;

  while (bWait == true) {
    std::this_thread::yield();
  }

  _failed = 0;
  _successful = 0;
  system_clock::time_point now = system_clock::now();

  do {
    DocBodies::const_iterator iBody = _iFirst;

    do {
      enum : long { WriteCreated = 201, WriteAccepted = 202 };
      VPack body = *iBody;
      doc.create(_collection, _connection, body);
      con.run();
      con.result(false);

      auto code = con.responseCode();
      if (code != WriteCreated && code != WriteAccepted) {
        ++_failed;
      } else {
        ++_successful;
      }
    } while (++iBody != _iEnd);
  } while (--loops);

  _duration =
      chrono::duration_cast<chrono::microseconds>(system_clock::now() - now);
}

void BucketWriteTest::setDocBodies(DocDatas::const_iterator iFirst,
                                   DocDatas::const_iterator iEnd) {
  using arangodb::velocypack::Builder;
  using arangodb::velocypack::Parser;
  using arangodb::velocypack::Options;

  while (iFirst != iEnd) {
    std::string const& body = *iFirst++;
    _bodies.emplace_back(Parser::fromJson(body)->steal());
  }

  _iFirst = _bodies.cbegin();
  _iEnd = _bodies.cend();
}
