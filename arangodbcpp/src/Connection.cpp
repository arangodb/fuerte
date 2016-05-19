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

#include "arangodbcpp/Connection.h"

#include <algorithm>
#include <velocypack/Builder.h>
#include <velocypack/Slice.h>
#include <velocypack/Parser.h>
#include <velocypack/Sink.h>
#include <velocypack/Dumper.h>
#include <velocypack/Value.h>

#include <curlpp/Infos.hpp>

#include "arangodbcpp/ConnectionBase.h"

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
      continue;
    }
    os << chr;
    old = chr;
  }
  os << "\"}";
  setBuffer(os.str());
  reset(err);
}

void Connection::addHeader(const ConOption& inp) {
  _headers.push_back(inp.headerString());
}

void Connection::addQuery(const ConOption& inp) {
  _queries += '&' + inp.queryString();
}

void Connection::addHeader(ConOption&& inp) {
  _headers.push_back(inp.headerString());
}

void Connection::addQuery(ConOption&& inp) {
  _queries += '&' + inp.queryString();
}

void Connection::setBuffer(const std::string& inp) {
  switch (_prot) {
    case Protocol::JSonVPack:
    case Protocol::JSon: {
      size_t len = inp.length();
      _buf.resize(len);
      memcpy(_buf.data(), inp.data(), len);
      break;
    }

    case Protocol::VPack:
    case Protocol::VPackJSon: {
      VPack data = vpack(reinterpret_cast<const uint8_t*>(inp.c_str()),
                         inp.length(), false);
      _buf.resize(data->size());
      memcpy(_buf.data(), data->data(), data->size());
    }
    default:;
  }
}

void Connection::setUrl(const Connection::Url& inp) {
#ifdef FUERTE_CONNECTIONURL
  std::string url = inp.httpUrl();
#else
  std::string url = inp;
#endif
  if (!_queries.empty()) {
    _queries[0] = '?';
    url += _queries;
  }
  setOpt(curlpp::options::Url(url));
}

void Connection::defaultContentType(Format inp) {
  if (inp == Format::JSon) {
    switch (_prot) {
      case Protocol::VPack: {
        _prot = Protocol::VPackJSon;
        break;
      }
      case Protocol::JSonVPack: {
        _prot = Protocol::JSon;
        break;
      }
      default:;
    }
    return;
  }
  switch (_prot) {
    case Protocol::JSon: {
      _prot = Protocol::JSonVPack;
      break;
    }
    case Protocol::VPackJSon: {
      _prot = Protocol::VPack;
      break;
    }
    default:;
  }
}

void Connection::defaultAccept(Format inp) {
  if (inp == Format::JSon) {
    switch (_prot) {
      case Protocol::VPack: {
        _prot = Protocol::JSonVPack;
        break;
      }
      case Protocol::VPackJSon: {
        _prot = Protocol::JSon;
        break;
      }
      default:;
    }
    return;
  }
  switch (_prot) {
    case Protocol::JSon: {
      _prot = Protocol::VPackJSon;
      break;
    }
    case Protocol::JSonVPack: {
      _prot = Protocol::VPack;
      break;
    }
    default:;
  }
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
  char* pData = reinterpret_cast<char*>(data->data());
  if (pData == nullptr) {
    return;
  }
  std::string field;
  switch (_prot) {
    case Protocol::VPack:
    case Protocol::JSonVPack: {
      field.append(pData, data->length());
      break;
    }

    case Protocol::JSon:
    case Protocol::VPackJSon: {
      field = json(data, false);
      break;
    }

    default: { return; }
  }
  setOpt(curlpp::options::PostFields(field));
  setOpt(curlpp::options::PostFieldSize(field.length()));
}

//
//
//
void Connection::setBuffer() {
  _buf.clear();
  setBuffer(this, &Connection::WriteMemoryCallback);
}

//
// Configures whether the next operation will be done
// synchronously or asyncronously
//
// IMPORTANT
// This should be the last configuration item to be set
//
void Connection::setAsynchronous(const bool bAsync) {
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

ConnectionBase& Connection::reset() {
  _headers.clear();
  _queries.clear();
  reset(Mode::SyncRun);
  return *this;
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
  long res = responseCode();
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
  if (inp.empty()) {
    return;
  }
  std::string field;
  switch (_prot) {
    case Protocol::VPack:
    case Protocol::JSonVPack: {
      const uint8_t* data = reinterpret_cast<const uint8_t*>(inp.c_str());
      VPack vp = vpack(data, inp.length(), false);
      data = vp->data();
      field.append(reinterpret_cast<const char*>(data), vp->length());
      break;
    }
    case Protocol::JSon:
    case Protocol::VPackJSon: {
      field = inp;
      break;
    }

    default: { return; }
  }
  setOpt(curlpp::options::PostFields(field));
  setOpt(curlpp::options::PostFieldSize(field.length()));
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
//      Curlpp callback function that receives the data returned
//      from the operation performed into the default write buffer
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

ConnectionBase::VPack Connection::fromVPData() const {
  if (_buf.empty()) {
    return VPack{};
  }
  VPack buf{std::make_shared<VBuffer>(_buf.size())};
  buf->append(&_buf[0], _buf.size());
  return buf;
}

ConnectionBase::VPack Connection::vpack(const uint8_t* data, std::size_t sz,
                                        bool bSort) {
  if (sz < 2) {
    return ConnectionBase::VPack{};
  }
  using arangodb::velocypack::Builder;
  using arangodb::velocypack::Parser;
  using arangodb::velocypack::Options;
  Options options;
  options.sortAttributeNames = bSort;
  Parser parser{&options};
  parser.parse(data, sz);
  std::shared_ptr<Builder> vp{parser.steal()};
  return vp->buffer();
}

//
// Converts JSon held in the default write buffer
// to a shared velocypack buffer
//
ConnectionBase::VPack Connection::fromJSon(const bool bSorted) const {
  return vpack(reinterpret_cast<const uint8_t*>(&_buf[0]), _buf.size(),
               bSorted);
}

ConnectionBase::VPack Connection::result(const bool bSort) const {
  switch (_prot) {
    case Protocol::VPack:
    case Protocol::VPackJSon: {
      return fromVPData();
    }

    case Protocol::JSonVPack:
    case Protocol::JSon: {
      return fromJSon(bSort);
    }

    default:;
  }
  return VPack{};
}

void Connection::httpProtocol() {
  if (_prot == Protocol::VPackJSon || _prot == Protocol::VPack) {
    addHeader(ConOption("Accept", "application/x-velocypack"));
  }
  if (_prot == Protocol::JSonVPack || _prot == Protocol::VPack) {
    addHeader(ConOption("content-type", "application/x-velocypack"));
  }
}

Connection::Connection() : _prot(Protocol::JSon) {}

Connection::~Connection() {}
}
}
