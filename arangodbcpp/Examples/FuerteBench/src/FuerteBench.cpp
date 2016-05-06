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

#include <iostream>
#include <sstream>
#include <array>
#include <algorithm>
#include <fstream>
#include <utility>
#include <thread>

namespace {
typedef std::array<std::string, 7> OptArray;
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
    "-h"  // Host address e.g http://localhost:8529
    ,
    "-r"  // ArangoDB to return VPack
}};

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

enum : uint16_t {
  Opt_FileName,
  Opt_Threads,
  Opt_Iterations,
  Opt_ColName,
  Opt_DbName,
  Opt_HostName,
  Opt_RetVPack
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
    : _argc(argc), _argv(argv), _loops(1), _threads(1) {}

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

std::string FuerteBench::collectionExists() const {
  BucketReadTest test{_hostName, _dbName, _colName};
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

void FuerteBench::createTestObjs() {
  BucketReadTest test{_hostName, _dbName, _colName};
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
      iFirst = test.setDocs(iFirst, bucket, _prot);
      _tests.push_back(test);
      test.isolate();
    } while (--extra);
    --bucket;
  }
  do {
    iFirst = test.setDocs(iFirst, bucket, _prot);
    _tests.push_back(test);
    test.isolate();
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
    std::string err = collectionExists();
    if (!err.empty()) {
      cout << err << endl;
      return false;
    }
  }
  if (!getDocNames()) {
    cout << _fileName << " file not found" << endl;
    return false;
  }
  if (_docNames.empty()) {
    cout << "No documents listed" << endl;
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
  using std::cout;
  using std::endl;
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
  cout << "All threads" << endl
       << "Successful reads   : " << readCount - missCount << endl
       << "Unsuccessful reads : " << missCount << endl
       << "Duration (usecs)   : " << _usecs.count() << endl
       << "Reads/sec          : " << (readCount * 1e6f / _usecs.count()) << endl
       << endl;
  ReadCounts::const_iterator iRead = readCounts.cbegin();
  ReadCounts::const_iterator iMiss = missCounts.cbegin();
  Durations::const_iterator iDuration = timesTaken.begin();
  BucketReadTest::ReadCount nThread = 1;
  do {
    readCount = *iRead;
    missCount = *iMiss;
    timeTaken = *iDuration;
    cout << "Thread : " << nThread << endl
         << "Successful reads   : " << readCount - missCount << endl
         << "Unsuccessful reads : " << missCount << endl
         << "Duration (usecs)   : " << timeTaken.count() << endl
         << "Reads/sec          : " << (readCount * 1e6f / timeTaken.count())
         << endl
         << endl;
    ++iMiss;
    ++iDuration;
    ++nThread;
  } while (++iRead != readCounts.cend());
}
