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

#include "FuerteBench.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <memory>
#include <utility>

#include "BucketReadTest.h"
#include "BucketWriteTest.h"

namespace {
typedef std::array<std::string, 8> OptArray;
const OptArray optStrs = {{
    "-f"  // File containing list of documents to load
    ,
    "-p"  // Number of threads
    ,
    "-m"  // Number of test iterations
    ,
    "-c"  // Name of the collection
    ,
    "-d"  // Name of the database
    ,
    "-h"  // Host address e.g http://127.0.0.1:8529
    ,
    "-r"  // ArangoDB to return VPack
    ,
    "-w"  // write tests
}};

enum : uint16_t {
  Opt_FileName,
  Opt_Threads,
  Opt_Iterations,
  Opt_ColName,
  Opt_DbName,
  Opt_HostName,
  Opt_RetVPack,
  Opt_Write
};

const std::string help{
    "Usage : FuerteBench [-f File-name] [-d Database-name] [-c "
    "Collection-name] [-h Host-url] "
    "[-p Number-of-threads] [-m number-of-iterations] [-r]\n\n"
    "-r Requests documents returned as VPack objects\n\n"
    "Bencbmarks the arangodbcpp library by reading multiple Documents from "
    "an ArangoDB database using multiple threads\n\n"
    "The file is a list of the Document names, (1 per line), to be read\n"
    "The default number of threads is 1, and will never exceed the number of "
    "documents to be read\n"
    "The default number of iterations is 1\n\n"
    "The File-name, Database-name and Collection-name are required for the "
    "test\n"};

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
      _dbName("_system"),
      _testCase(TestCase::READ) {}

bool FuerteBench::readDocDatas() {
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
          _docDatas.push_back(std::move(line));
        }
      }
    }

    return true;
  }

  return false;
}

void FuerteBench::processCmdLine() {
  _prot = Protocol::JSon;
  for (int idx = 1; idx < _argc; ++idx) {
    const std::string opt{_argv[idx]};
    OptArray::const_iterator iOpt =
        std::find(optStrs.begin(), optStrs.end(), opt);

    switch (iOpt - optStrs.begin()) {
      case Opt_RetVPack: {
        _prot = Protocol::VPackJSon;
        break;
      }
      case Opt_Write: {
        _testCase = TestCase::WRITE;
        break;
      }
      case Opt_FileName: {
        if (idx + 1 == _argc) {
          break;
        }
        _fileName = _argv[++idx];
        break;
      }
      case Opt_HostName: {
        if (idx + 1 == _argc) {
          break;
        }
        _hostName = _argv[++idx];
        break;
      }
      case Opt_DbName: {
        if (idx + 1 == _argc) {
          break;
        }
        _dbName = _argv[++idx];
        break;
      }
      case Opt_ColName: {
        if (idx + 1 == _argc) {
          break;
        }
        _colName = _argv[++idx];
        break;
      }
      case Opt_Threads: {
        if (idx + 1 == _argc) {
          break;
        }
        _threads = getNumber(_argv[++idx]);
        break;
      }
      case Opt_Iterations: {
        if (idx + 1 == _argc) {
          break;
        }
        _loops = getNumber(_argv[++idx]);
        break;
      }
      default:;
    }
  }
}

std::string FuerteBench::checkCollection() const {
  BucketReadTest test{_hostName, _dbName, _colName, _prot};

  if (!test.serverExists()) {
    return std::string{"Cannot connect to Server"};
  }

  if (!test.databaseExists()) {
    return std::string{"Cannot find the Database : "} + _dbName;
  }

  if (!test.collectionExists()) {
    return std::string{"Cannot find the Collection : "} + _colName;
  }

  return std::string{};
}

void FuerteBench::createTestObjs(
    std::function<std::unique_ptr<BucketTest>(
        DocDatas::const_iterator, DocDatas::const_iterator)> testFactory) {
  _tests.clear();

  const DocDatas::size_type sz = _docDatas.size();

  if (_threads > sz) {
    _threads = sz;
  }

  DocDatas::size_type bucket = sz / _threads;
  DocDatas::const_iterator iFirst = _docDatas.cbegin();
  const DocDatas::const_iterator iEnd = _docDatas.cend();
  DocDatas::size_type extra = sz - bucket * _threads;

  if (extra) {
    ++bucket;

    do {
      DocDatas::const_iterator iNext = iFirst + bucket;

      auto test = testFactory(iFirst, iNext);
      _tests.push_back(test.release());

      iFirst = iNext;
    } while (--extra);

    --bucket;
  }

  do {
    DocDatas::const_iterator iNext = iFirst + bucket;

    auto test = testFactory(iFirst, iNext);
    _tests.push_back(test.release());

    iFirst = iNext;
  } while (iFirst != iEnd);
}

bool FuerteBench::start() {
  using std::cout;
  using std::endl;
  namespace chrono = std::chrono;
  using system_clock = chrono::system_clock;
  processCmdLine();

  if (_fileName.empty() || _colName.empty() || _dbName.empty()) {
    cout << help << std::flush;
    return false;
  }

  {
    std::string err = checkCollection();

    if (!err.empty()) {
      cout << err << endl;
      return false;
    }
  }

  if (!readDocDatas()) {
    cout << _fileName << " file not found" << endl;
    return false;
  }

  if (_docDatas.empty()) {
    cout << "No documents listed" << endl;
    return false;
  }

  auto hostName = _hostName;
  auto dbName = _dbName;
  auto colName = _colName;
  auto prot = _prot;

  switch (_testCase) {
    case TestCase::READ:
      std::cout << "READ TEST\n=========\n" << endl;

      createTestObjs([hostName, dbName, colName, prot](
          DocDatas::const_iterator iStart, DocDatas::const_iterator iEnd) {
        std::unique_ptr<BucketReadTest> test(
            new BucketReadTest(hostName, dbName, colName, prot));

        test->setDocNames(iStart, iEnd);
        return test;
      });
      break;

    case TestCase::WRITE:
      std::cout << "WRITE TEST\n==========\n" << endl;

      createTestObjs([hostName, dbName, colName, prot](
          DocDatas::const_iterator iStart, DocDatas::const_iterator iEnd) {
        std::unique_ptr<BucketWriteTest> test(
            new BucketWriteTest(hostName, dbName, colName, prot));

        test->setDocBodies(iStart, iEnd);
        return test;
      });
      break;
  }

  std::vector<std::thread> threads;
  threads.reserve(_docDatas.size());
  std::atomic_bool bWait{true};

  for (BucketTest* test : _tests) {
    threads.push_back(std::thread{std::ref(*test), std::ref(bWait), _loops});
  }

  system_clock::time_point now = system_clock::now();
  bWait = false;

  for (std::thread& pth : threads) {
    pth.join();
  }

  _duration =
      chrono::duration_cast<chrono::microseconds>(system_clock::now() - now);

  return true;
}

void FuerteBench::outputReport() const {
  typedef std::vector<BucketTest::TestCount> TestCounts;
  typedef std::vector<std::chrono::microseconds> Durations;
  using std::cout;
  using std::endl;

  TestCounts successfulCounts;
  TestCounts failedCounts;
  Durations timesTaken;
  BucketTest::TestCount successfulCount{0};
  BucketTest::TestCount failedCount{0};
  std::chrono::microseconds timeTaken{0};

  successfulCounts.reserve(_tests.size());
  timesTaken.reserve(_tests.size());

  for (const BucketTest* test : _tests) {
    BucketTest::TestCount successful = test->successful();
    successfulCount += successful;
    successfulCounts.push_back(successful);

    BucketTest::TestCount failed = test->failed();
    failedCounts.push_back(failed);
    failedCount += failed;

    std::chrono::microseconds duration = test->duration();
    timesTaken.push_back(duration);
  }

  cout << "All threads" << endl
       << "Successful reads   : " << successfulCount << endl
       << "Unsuccessful reads : " << failedCount << endl
       << "Duration (secs)    : " << (_duration.count() / 1e6f) << endl
       << "Ops/sec            : "
       << ((successfulCount + failedCount) * 1e6f / _duration.count()) << endl
       << endl;

  TestCounts::const_iterator iSuccessful = successfulCounts.cbegin();
  TestCounts::const_iterator iFailed = failedCounts.cbegin();
  Durations::const_iterator iDuration = timesTaken.begin();
  BucketTest::TestCount nThread = 1;

  do {
    successfulCount = *iSuccessful;
    failedCount = *iFailed;
    timeTaken = *iDuration;
    cout << "Thread             : " << nThread << endl
         << "Ops/sec            : "
         << ((successfulCount + failedCount) * 1e6f / _duration.count()) << endl
         << endl;

    ++iFailed;
    ++iDuration;
    ++nThread;

  } while (++iSuccessful != successfulCounts.cend());
}
