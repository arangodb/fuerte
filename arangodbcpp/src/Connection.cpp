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
#include <cstring>
#include <algorithm>
#include <velocypack/Builder.h>
#include <velocypack/Slice.h>
#include <velocypack/Parser.h>
#include <velocypack/Sink.h>
#include <velocypack/Dumper.h>
#include <velocypack/Value.h>

#include <curlpp/Infos.hpp>

#include "arangodbcpp/Connection.h"

namespace arangodb {

namespace dbinterface {

//
// Flags an error has occured and transfers the error message
// to the default write buffer
//
void Connection::errFound(const std::string& inp, const Mode err) {
  std::ostringstream os;
  char old = '\0';
  os << "{ \"errorMessage\":\"";
  // Escape double quotes in the message
  for (char chr : inp) {
    if (old != '\\' && chr == '"') {
      os << "\\\"";
      old = '\0';
      continue;
    }
    os << chr;
    old = chr;
  }
  os << "\"}";
  setBuffer(os.str());
  reset(err);
}

void Connection::fixProtocol(std::string& url) {
  typedef std::string::size_type size_type;
  std::string sep{'/', '/'};
  size_type len = url.find(sep);
  if (len == std::string::npos) {
    if (!url.empty()) {
      url.insert(0, "http://");
    }
    return;
  }
  std::string prot = url.substr(0, len);
  std::transform(prot.begin(), prot.end(), prot.begin(), ::tolower);
  url = url.substr(len);
  if (prot == "http+ssl:" || prot == "https:") {
    url = "https:" + url;
    return;
  }
  url = "http:" + url;
}

std::string Connection::json(const VPack& v, bool bSort) {
  using arangodb::velocypack::Slice;
  using arangodb::velocypack::Dumper;
  using arangodb::velocypack::StringSink;
  using arangodb::velocypack::Options;
  Slice slice{v->data()};
  std::string tmp;
  StringSink sink(&tmp);
  Options opts;
  opts.sortAttributeNames = bSort;
  Dumper::dump(slice, &sink, &opts);
  return tmp;
}

void Connection::setPostField(const VPack data) {
  std::string field{"{}"};
  if (data.get() != nullptr) {
    field = json(data, false);
  }
  setPostField(field);
}

//
//
//
void Connection::setBuffer() {
  _buf.clear();
  setBuffer(this, &Connection::WriteMemoryCallback);
}

//
//
//
void Connection::setJsonContent(HttpHeaderList& headers) {
  headers.push_back("Content-Type: application/json");
}

//
// Configures whether the next operation will be done
// synchronously or asyncronously
//
// IMPORTANT
// This should be the last configuration item to be set
//
void Connection::setSync(const bool bAsync) {
  if (bAsync) {
    _mode = Mode::AsyncRun;
    _async.add(&_request);
    return;
  }
  _mode = Mode::SyncRun;
}

void Connection::reset(const Mode inp) {
  if (_mode == Mode::AsyncRun) {
    _async.remove(&_request);
  }
  _request.reset();
  _mode = inp;
}

void Connection::runAgain(bool bAsync) {
  if (_mode == Mode::Done) {
    _buf.clear();
    if (bAsync) {
      _mode = Mode::AsyncRun;
      _async.add(&_request);
      asyncRun();
      if (_mode == Mode::AsyncRun) {
        return;
      }
    } else {
      _mode = Mode::SyncRun;
      syncRun();
    }
    if (_buf.empty()) {
      httpResponse();
    }
  }
}

void Connection::run() {
  switch (_mode) {
    case Mode::AsyncRun: {
      asyncRun();
      if (_mode != Mode::AsyncRun) {
        break;
      }
      // Deliberately drops through
    }
    default: { return; }
    case Mode::SyncRun: {
      syncRun();
      break;
    }
  }
  if (_buf.empty()) {
    httpResponse();
  }
}

//
// Synchronous operation which will complete before returning
//
void Connection::syncRun() {
  try {
    _request.perform();
    _mode = Mode::Done;
  } catch (curlpp::LogicError& e) {
    errFound(e.what(), Mode::LogicError);
    return;
  } catch (curlpp::LibcurlRuntimeError& e) {
    errFound(e.what());
    return;
  } catch (curlpp::RuntimeError& e) {
    errFound(e.what());
    return;
  }
}

//
// Asynchronous operation which may need to be run multiple times
// before completing
//
void Connection::asyncRun() {
  try {
    {
      int nLeft = 1;
      if (!_async.perform(&nLeft)) {
        return;
      }
      if (!nLeft) {
        _async.remove(&_request);
        _mode = Mode::Done;
        return;
      }
    }
    {
      struct timeval timeout;
      int rc;  // select() return code
      fd_set fdread;
      fd_set fdwrite;
      fd_set fdexcep;
      int maxfd;
      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);
      FD_ZERO(&fdexcep);
      // set a suitable timeout to play around with
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      // get file descriptors from the transfers
      _async.fdset(&fdread, &fdwrite, &fdexcep, &maxfd);
      rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
      if (rc == -1) {
        errFound("Asynchronous select error");
      }
    }
  } catch (curlpp::LogicError& e) {
    errFound(e.what(), Mode::LogicError);
    return;
  } catch (curlpp::LibcurlRuntimeError& e) {
    errFound(e.what());
    return;
  } catch (curlpp::RuntimeError& e) {
    errFound(e.what());
    return;
  }
}

void Connection::httpResponse() {
  long res = httpResponseCode();
  if (!res && _mode == Mode::AsyncRun) {
    typedef curlpp::Multi::Msgs M_Msgs;
    M_Msgs msgs = _async.info();
    CURLcode code = msgs.front().second.code;
    std::string msg{curl_easy_strerror(code)};
    errFound(msg);
    return;
  }
  {
    // Create a JSon response
    std::ostringstream os;
    os << "{\"result\":true,\"error\":false,\"code\":" << res << "}";
    setBuffer(os.str());
  }
}

void Connection::setPostField(const std::string& inp) {
  setOpt(curlpp::options::PostFields(inp));
  setOpt(curlpp::options::PostFieldSize(inp.length()));
}

//
// Sets the curlpp callback function that receives the data returned
// from the operation performed
//
void Connection::setBuffer(size_t (*f)(char* p, size_t sz, size_t m)) {
  curlpp::types::WriteFunctionFunctor fnc(f);
  setOpt(curlpp::options::WriteFunction(fnc));
}

//
//	Curlpp callback function that receives the data returned
//	from the operation performed into the default write buffer
//
size_t Connection::WriteMemoryCallback(char* ptr, size_t size, size_t nmemb) {
  size_t realsize = size * nmemb;
  if (realsize != 0) {
    size_t offset = _buf.size();
    _buf.resize(offset + realsize);
    memcpy(&_buf[offset], ptr, realsize);
  }
  return realsize;
}

//
// Converts JSon held in the default write buffer
// to a shared velocypack buffer
//
Connection::VPack Connection::fromJSon(const bool bSorted) const {
  if (!_buf.empty()) {
    using arangodb::velocypack::Builder;
    using arangodb::velocypack::Parser;
    using arangodb::velocypack::Options;
    Options options;
    options.sortAttributeNames = bSorted;
    Parser parser{&options};
    parser.parse(&_buf[0], _buf.size());
    std::shared_ptr<Builder> vp{parser.steal()};
    return vp->buffer();
  }
  return VPack{new VBuffer()};
}

Connection::Connection() {}

Connection::~Connection() {}
}
}
