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
#include <iostream>
#include <sstream>
#include <array>
#include <algorithm>
#include <fstream>
#include <utility>
#include <thread>

#include "FuerteBench.h"

namespace {
typedef std::array<std::string, 5> OptArray;
const OptArray optStrs = {
    "-f"  // File containing list of documents to load
    ,
    "-p"  // Number of threads
    ,
    "-m"  // Number of test iterations
    ,
    "-c"  // Name of the collection
    ,
    "-d"  // Name of the database
};

enum : uint16_t {
  Opt_FileName,
  Opt_Threads,
  Opt_Iterations,
  Opt_ColName,
  Opt_DbName
};

uint16_t getNumber(const char* inp) {
  std::istringstream is{std::string{inp}};
  uint16_t res = 1;
  is >> res;
  if (!res) {
    res = 1;
  }
  return res;
}
}

FuerteBench::FuerteBench(int argc, const char* argv[])
    : _argc(argc),
      _argv(argv),
      _loops(1),
      _threads(1),
      _dbName{"internal"},
      _colName{"Test"},
      _fileName{"ids.csv"} {}

bool FuerteBench::getDocNames() {
  static std::string wspc{" \t\r\n\f\v"};
  std::ifstream input{_fileName};
  if (input.is_open()) {
    while (!input.eof()) {
      std::string line;
      std::getline(input, line);
      if (!line.empty()) {
        std::string::size_type p1 = line.find_first_not_of(wspc);
        if (p1 != std::string::npos) {
          std::string::size_type p2 = line.find_last_not_of(wspc) + 1;
          line = line.substr(p1, p2 - p1);
          _docNames.push_back(std::move(line));
        }
      }
    }
    return true;
  }
  return false;
}

void FuerteBench::processCmdLine() {
  int idx = 1;
  for (; idx + 1 < _argc; ++idx) {
    const std::string opt{_argv[idx]};
    OptArray::const_iterator iOpt =
        std::find(optStrs.begin(), optStrs.end(), opt);
    switch (iOpt - optStrs.begin()) {
      case Opt_FileName: {
        _fileName = _argv[++idx];
        break;
      }
      case Opt_DbName: {
        _dbName = _argv[++idx];
        break;
      }
      case Opt_ColName: {
        _colName = _argv[++idx];
        break;
      }
      case Opt_Threads: {
        _threads = getNumber(_argv[++idx]);
        break;
      }
      case Opt_Iterations: {
        _loops = getNumber(_argv[++idx]);
        break;
      }
      default:;
    }
  }
}

bool FuerteBench::collectionExists() const {
  BucketReadTest test{_dbName, _colName};
  return test.collectionExists();
}

void FuerteBench::createTestObjs() {
  BucketReadTest test{_dbName, _colName};
  _tests.clear();
  const DocNames::size_type sz = _docNames.size();
  if (_threads > sz) {
    _threads = sz;
  }
  DocNames::size_type bucket = sz / _threads;
  DocNames::const_iterator iFirst = _docNames.cbegin();
  const DocNames::const_iterator iEnd = _docNames.cend();
  DocNames::size_type extra = sz - bucket * _threads;
  if (extra) {
    ++bucket;
    do {
      iFirst = test.setDocs(iFirst, bucket);
      _tests.push_back(test);
      test.isolate();
    } while (--extra);
    --bucket;
  }
  do {
    iFirst = test.setDocs(iFirst, bucket);
    _tests.push_back(test);
    test.isolate();
  } while (iFirst != iEnd);
}

bool FuerteBench::start() {
  namespace chrono = std::chrono;
  using system_clock = chrono::system_clock;
  processCmdLine();
  if (!collectionExists()) {
    std::cout << "Collection not found" << std::endl;
    std::cout << "Collection : " << _colName << std::endl;
    std::cout << "Database   : " << _dbName << std::endl;
    return false;
  }
  if (!getDocNames()) {
    std::cout << _fileName << " file not found" << std::endl;
    return false;
  }
  if (_docNames.empty()) {
    std::cout << "No documents listed" << std::endl;
    return false;
  }
  createTestObjs();
  std::vector<std::thread> threads;
  threads.reserve(_docNames.size());
  std::atomic_bool bWait{true};
  for (BucketReadTest& ref : _tests) {
    threads.push_back(std::thread{std::ref(ref), std::ref(bWait), _loops});
  }
  bWait = false;
  system_clock::time_point now = system_clock::now();
  for (std::thread& pth : threads) {
    pth.join();
  }
  _usecs =
      chrono::duration_cast<chrono::microseconds>(system_clock::now() - now);

  return true;
}

void FuerteBench::outputReport() const {
  typedef std::vector<BucketReadTest::ReadCount> ReadCounts;
  typedef std::vector<std::chrono::microseconds> Durations;
  ReadCounts readCounts;
  ReadCounts missCounts;
  Durations timesTaken;
  BucketReadTest::ReadCount readCount{0};
  BucketReadTest::ReadCount missCount{0};
  std::chrono::microseconds timeTaken{0};
  readCounts.reserve(_tests.size());
  timesTaken.reserve(_tests.size());
  for (const BucketReadTest& test : _tests) {
    BucketReadTest::ReadCount reads = test.reads();
    std::chrono::microseconds duration = test.duration();
    readCount += reads;
    readCounts.push_back(reads);
    timesTaken.push_back(duration);
    reads = test.misses();
    missCounts.push_back(reads);
    missCount += reads;
  }
  std::cout << "All threads" << std::endl;
  std::cout << "Successful reads   : " << readCount - missCount << std::endl;
  std::cout << "Unsuccessful reads : " << missCount << std::endl;
  std::cout << "Duration (usecs)   : " << _usecs.count() << std::endl;
  std::cout << "Reads/sec          : " << (readCount * 1e6f / _usecs.count())
            << std::endl
            << std::endl;
  ReadCounts::const_iterator iRead = readCounts.cbegin();
  ReadCounts::const_iterator iMiss = missCounts.cbegin();
  Durations::const_iterator iDuration = timesTaken.begin();
  BucketReadTest::ReadCount nThread = 1;
  do {
    readCount = *iRead;
    missCount = *iMiss;
    timeTaken = *iDuration;
    std::cout << "Thread : " << nThread << std::endl;
    std::cout << "Successful reads   : " << readCount - missCount << std::endl;
    std::cout << "Unsuccessful reads : " << missCount << std::endl;
    std::cout << "Duration (usecs)   : " << timeTaken.count() << std::endl;
    std::cout << "Reads/sec          : "
              << (readCount * 1e6f / timeTaken.count()) << std::endl
              << std::endl;
    ++iMiss;
    ++iDuration;
    ++nThread;
  } while (++iRead != readCounts.cend());
}
