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

#include <fuerte/HttpConnection.h>

#include <algorithm>

#include <curlpp/Infos.hpp>

#include <velocypack/Builder.h>
#include <velocypack/Dumper.h>
#include <velocypack/Parser.h>
#include <velocypack/Sink.h>
#include <velocypack/Slice.h>
#include <velocypack/Value.h>

namespace arangodb {
namespace dbinterface {

// Flags an error has occured and transfers the error message
// to the default write buffer
void HttpConnection::errFound(const std::string& inp, const Mode err) {
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

void HttpConnection::addHeader(const ConOption& inp) {
  _headers.push_back(inp.headerString());
}

void HttpConnection::addQuery(const ConOption& inp) {
  _queries += '&' + inp.queryString();
}

void HttpConnection::addHeader(ConOption&& inp) {
  _headers.push_back(inp.headerString());
}

void HttpConnection::addQuery(ConOption&& inp) {
  _queries += '&' + inp.queryString();
}

void HttpConnection::setBuffer(const std::string& inp) {
  switch (_prot) {
    case Protocol::JsonVPack:
    case Protocol::Json: {
      size_t len = inp.length();
      _buf.resize(len);
      memcpy(_buf.data(), inp.data(), len);
      break;
    }

    case Protocol::VPack:
    case Protocol::VPackJson: {
      VPack data =
          vpack(reinterpret_cast<const uint8_t*>(inp.c_str()), inp.length());
      _buf.resize(data->size());
      memcpy(_buf.data(), data->data(), data->size());
    }
    default:;
  }
}

void HttpConnection::setUrl(const HttpConnection::Url& inp) {
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

void HttpConnection::defaultContentType(Format inp) {
  if (inp == Format::Json) {
    switch (_prot) {
      case Protocol::VPack: {
        _prot = Protocol::VPackJson;
        break;
      }
      case Protocol::JsonVPack: {
        _prot = Protocol::Json;
        break;
      }
      default:;
    }
    return;
  }
  switch (_prot) {
    case Protocol::Json: {
      _prot = Protocol::JsonVPack;
      break;
    }
    case Protocol::VPackJson: {
      _prot = Protocol::VPack;
      break;
    }
    default:;
  }
}

void HttpConnection::defaultAccept(Format inp) {
  if (inp == Format::Json) {
    switch (_prot) {
      case Protocol::VPack: {
        _prot = Protocol::JsonVPack;
        break;
      }
      case Protocol::VPackJson: {
        _prot = Protocol::Json;
        break;
      }
      default:;
    }
    return;
  }
  switch (_prot) {
    case Protocol::Json: {
      _prot = Protocol::VPackJson;
      break;
    }
    case Protocol::JsonVPack: {
      _prot = Protocol::VPack;
      break;
    }
    default:;
  }
}

std::string HttpConnection::json(const VPack& v) {
  using arangodb::velocypack::Slice;
  using arangodb::velocypack::Dumper;
  using arangodb::velocypack::StringSink;
  Slice slice{v->data()};
  std::string tmp;
  StringSink sink(&tmp);
  Dumper::dump(slice, &sink);
  return tmp;
}

void HttpConnection::setPostField(const VPack data) {
  char* pData = reinterpret_cast<char*>(data->data());
  if (pData == nullptr) {
    return;
  }
  std::string field;
  switch (_prot) {
    case Protocol::VPack:
    case Protocol::JsonVPack: {
      field.append(pData, data->length());
      break;
    }

    case Protocol::Json:
    case Protocol::VPackJson: {
      field = json(data);
      break;
    }

    default: { return; }
  }
  setOpt(curlpp::options::PostFields(field));
  setOpt(curlpp::options::PostFieldSize(field.length()));
}

void HttpConnection::setBuffer() {
  _buf.clear();
  setBuffer(this, &HttpConnection::WriteMemoryCallback);
}

// Configures whether the next operation will be done
// synchronously or asyncronously
//
// IMPORTANT
// This should be the last configuration item to be set
void HttpConnection::setAsynchronous(const bool bAsync) {
  if (bAsync) {
    _mode = Mode::AsyncRun;
    _async.add(&_request);
    return;
  }
  _mode = Mode::SyncRun;
}

void HttpConnection::reset(const Mode inp) {
  if (_mode == Mode::AsyncRun) {
    _async.remove(&_request);
  }
  _request.reset();
  _mode = inp;
}

void HttpConnection::runAgain(bool bAsync) {
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

void HttpConnection::run() {
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

Connection& HttpConnection::reset() {
  _headers.clear();
  _queries.clear();
  reset(Mode::SyncRun);
  return *this;
}

// Synchronous operation which will complete before returning
void HttpConnection::syncRun() {
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

// Asynchronous operation which may need to be run multiple times
// before completing
void HttpConnection::asyncRun() {
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

void HttpConnection::httpResponse() {
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
    // Create a Json response
    std::ostringstream os;
    os << "{\"result\":true,\"error\":false,\"code\":" << res << "}";
    setBuffer(os.str());
  }
}

void HttpConnection::setPostField(const std::string& inp) {
  if (inp.empty()) {
    return;
  }
  std::string field;
  switch (_prot) {
    case Protocol::VPack:
    case Protocol::JsonVPack: {
      const uint8_t* data = reinterpret_cast<const uint8_t*>(inp.c_str());
      VPack vp = vpack(data, inp.length());
      data = vp->data();
      field.append(reinterpret_cast<const char*>(data), vp->length());
      break;
    }
    case Protocol::Json:
    case Protocol::VPackJson: {
      field = inp;
      break;
    }

    default: { return; }
  }
  setOpt(curlpp::options::PostFields(field));
  setOpt(curlpp::options::PostFieldSize(field.length()));
}

// Sets the curlpp callback function that receives the data returned
// from the operation performed
void HttpConnection::setBuffer(size_t (*f)(char* p, size_t sz, size_t m)) {
  curlpp::types::WriteFunctionFunctor fnc(f);
  setOpt(curlpp::options::WriteFunction(fnc));
}

//      Curlpp callback function that receives the data returned
//      from the operation performed into the default write buffer
size_t HttpConnection::WriteMemoryCallback(char* ptr, size_t size,
                                           size_t nmemb) {
  size_t realsize = size * nmemb;
  if (realsize != 0) {
    size_t offset = _buf.size();
    _buf.resize(offset + realsize);
    memcpy(&_buf[offset], ptr, realsize);
  }
  return realsize;
}

Connection::VPack HttpConnection::fromVPData() const {
  if (_buf.empty()) {
    return VPack{};
  }
  VPack buf{std::make_shared<VBuffer>(_buf.size())};
  buf->append(&_buf[0], _buf.size());
  return buf;
}

Connection::VPack HttpConnection::vpack(const uint8_t* data, std::size_t sz) {
  if (sz < 2) {
    return Connection::VPack{};
  }
  using arangodb::velocypack::Builder;
  using arangodb::velocypack::Parser;
  Parser parser;
  parser.parse(data, sz);
  std::shared_ptr<Builder> vp{parser.steal()};
  return vp->buffer();
}

// Converts Json held in the default write buffer
// to a shared velocypack buffer
Connection::VPack HttpConnection::fromJson() const {
  return vpack(reinterpret_cast<const uint8_t*>(&_buf[0]), _buf.size());
}

Connection::VPack HttpConnection::result() const {
  switch (_prot) {
    case Protocol::VPack:
    case Protocol::VPackJson: {
      return fromVPData();
    }

    case Protocol::JsonVPack:
    case Protocol::Json: {
      return fromJson();
    }

    default:;
  }
  return VPack{};
}

void HttpConnection::httpProtocol() {
  if (_prot == Protocol::VPackJson || _prot == Protocol::VPack) {
    addHeader(ConOption("Accept", "application/x-velocypack"));
  }
  if (_prot == Protocol::JsonVPack || _prot == Protocol::VPack) {
    addHeader(ConOption("content-type", "application/x-velocypack"));
  }
}

HttpConnection::HttpConnection() : _prot(Protocol::Json) {}

HttpConnection::~HttpConnection() {}
}
}
