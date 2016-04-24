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

#include <string>
#include <vector>

#include "BucketReadTest.h"

class FuerteBench {
 public:
  typedef BucketReadTest::DocNames DocNames;
  typedef BucketReadTest::LoopCount LoopCount;
  typedef uint16_t ThreadCount;
  typedef std::vector<BucketReadTest> TestObjs;
  FuerteBench(int argc, const char* argv[]);
  static std::string hostUrl();
  static uint16_t hostPort();
  bool start();
  void outputReport() const;

 private:
  void processCmdLine();
  bool getDocNames();
  void createTestObjs();
  std::string collectionExists() const;

  int _argc;
  const char** _argv;
  LoopCount _loops;
  ThreadCount _threads;
  std::string _hostName;
  std::string _dbName;
  std::string _colName;
  std::string _fileName;
  DocNames _docNames;
  TestObjs _tests;
  std::chrono::microseconds _usecs;
};

inline std::string FuerteBench::hostUrl() { return "localhost"; }

inline uint16_t FuerteBench::hostPort() { return 8529; }

#endif  // FUERTEBENCH_H
