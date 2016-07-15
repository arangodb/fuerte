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

#ifndef FUERTEBENCH_H
#define FUERTEBENCH_H

#include <functional>
#include <string>
#include <vector>

#include "BucketTest.h"

class FuerteBench {
 public:
  typedef BucketTest::DocDatas DocDatas;
  typedef BucketTest::LoopCount LoopCount;
  typedef uint16_t ThreadCount;
  typedef std::vector<BucketTest*> TestObjs;

  enum class TestCase { READ, WRITE };

 public:
  static std::string hostUrl();

 public:
  FuerteBench(int argc, const char* argv[]);

  bool start();
  void outputReport() const;

 private:
  typedef arangodb::dbinterface::Connection::Protocol Protocol;

  void processCmdLine();
  bool readDocDatas();
  void createTestObjs(std::function<std::unique_ptr<BucketTest>(
                          DocDatas::const_iterator, DocDatas::const_iterator)>);
  std::string checkCollection() const;

  int _argc;
  const char** _argv;
  LoopCount _loops;
  ThreadCount _threads;
  std::string _hostName;
  std::string _dbName;
  std::string _colName;
  std::string _fileName;
  DocDatas _docDatas;
  TestCase _testCase;
  TestObjs _tests;
  Protocol _prot;
  std::chrono::microseconds _duration;
};

inline std::string FuerteBench::hostUrl() { return "http://127.0.0.1:8529"; }

#endif
