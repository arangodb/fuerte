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

#include "BucketTest.h"

#include <atomic>

class BucketReadTest : public BucketTest {
 public:
  typedef std::vector<std::string> DocNames;

  BucketReadTest() = delete;
  BucketReadTest(const std::string& hostName, const std::string& dbName,
                 const std::string& colName, Connection::Protocol prot);

  void setDocNames(DocDatas::const_iterator iFirst,
                   DocDatas::const_iterator iEnd);

  bool collectionExists();
  bool databaseExists();
  bool serverExists();

  void operator()(std::atomic_bool& bWait, LoopCount loops) override final;

 private:
  Document::SPtr _document;
  DocNames::const_iterator _iFirst;
  DocNames::const_iterator _iEnd;
};

inline void BucketReadTest::setDocNames(DocDatas::const_iterator iFirst,
                                        DocDatas::const_iterator iEnd) {
  _iFirst = iFirst;
  _iEnd = iEnd;
}

#endif
