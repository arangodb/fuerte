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

#ifndef BUCKETVERSIONTEST_H
#define BUCKETVERSIONTEST_H

#include "BucketTest.h"

#include <atomic>

class BucketVersionTest : public BucketTest {
 public:
  BucketVersionTest() = delete;
  BucketVersionTest(const std::string& hostName, Connection::Protocol prot);

  bool collectionExists() { return true; }
  bool databaseExists() { return true; }
  bool serverExists();

  void operator()(std::atomic_bool& bWait, LoopCount loops) override final;
};

#endif
