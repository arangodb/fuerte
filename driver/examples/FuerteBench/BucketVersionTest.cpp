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

#include "BucketVersionTest.h"

#include <thread>

#include "FuerteBench.h"

BucketVersionTest::BucketVersionTest(const std::string& hostName,
                               Connection::Protocol prot)
    : BucketTest(hostName, "", "", prot) {}

bool BucketVersionTest::serverExists() {
  enum : long { ReadSuccess = 200 };
  Connection& con = *_connection;
  _server->version(_connection);
  con.run();
  return con.responseCode() == ReadSuccess;
}

void BucketVersionTest::operator()(std::atomic_bool& bWait, LoopCount loops) {
  namespace chrono = std::chrono;
  using system_clock = chrono::system_clock;
  Connection& con = *_connection;

  while (bWait == true) {
    std::this_thread::yield();
  }

  _failed = 0;
  _successful = 0;
  system_clock::time_point now = system_clock::now();

  do {
    enum : long { ReadSuccess = 200 };

    Connection& con = *_connection;
    _server->version(_connection);
    con.run();

    if (con.responseCode() != ReadSuccess) {
      ++_failed;
    } else {
      ++_successful;
    }
  } while (--loops);

  _duration =
      chrono::duration_cast<chrono::microseconds>(system_clock::now() - now);
}
