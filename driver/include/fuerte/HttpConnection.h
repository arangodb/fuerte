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

#ifndef FUERTE_HTTPCONNECTION_H
#define FUERTE_HTTPCONNECTION_H 1

#include <fuerte/Connection.h>

#include <list>
#include <vector>

#include <curlpp/Easy.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Multi.hpp>
#include <curlpp/Options.hpp>

namespace arangodb {
namespace dbinterface {
//  Provide network connection to Arango database
//  Hide the underlying use of cURL for implementation
class HttpConnection : public Connection {
 public:
  typedef std::shared_ptr<HttpConnection> SPtr;
  typedef std::list<std::string> HeaderList;
  typedef std::string QueryList;
  HttpConnection();
  virtual ~HttpConnection();
  Connection& operator=(const Protocol in);
  void addHeader(const ConOption& inp);
  void addQuery(const ConOption& inp);
  void addHeader(ConOption&& inp);
  void addQuery(ConOption&& inp);
  void defaultContentType(Format inp);
  void defaultAccept(Format inp);
  void httpProtocol();
  std::string httpUrl() const;
  void setUrl(const Url& inp);
  void setErrBuf(char* inp);
  void setPostField(const std::string& inp);
  void setPostField(const VPack data);
  void setHeaderOpts();
  void setCustomReq(const std::string inp);
  void setPostReq();
  // void setHeadReq();
  void setDeleteReq();
  void setGetReq();
  void setPutReq();
  void setPatchReq();
  void setVerbose(const bool inp);
  Connection& reset();
  template <typename T>
  void setBuffer(T* p, size_t (T::*f)(char* p, size_t sz, size_t m));
  void setBuffer(size_t (*f)(char* p, size_t sz, size_t m));
  void setBuffer();
  long responseCode();
  std::string httpEffectiveUrl();

  const std::string bufString() const;
  VPack result() const;
  VPack fromVPData() const;
  VPack fromJson() const;
  void setAsynchronous(const bool bAsync = false);
  void run();
  void runAgain(bool bAsync);
  bool isError() const;
  bool isRunning() const;
  bool bufEmpty() const;

 private:
  typedef std::vector<char> ChrBuf;
  size_t WriteMemoryCallback(char* ptr, size_t size, size_t nmemb);
  template <typename T>
  void setOpt(const T& inp);
  void asyncRun();
  void reset(const Mode inp);
  void syncRun();
  void errFound(const std::string& inp, const Mode err = Mode::RunError);
  void httpResponse();
  void setBuffer(const std::string& inp);

  HeaderList _headers;
  QueryList _queries;
  curlpp::Easy _request;
  curlpp::Multi _async;
  ChrBuf _buf;
  Mode _mode;
  Protocol _prot;
};

template <typename T>
inline void HttpConnection::setBuffer(T* p,
                                      size_t (T::*f)(char*, size_t, size_t)) {
  curlpp::types::WriteFunctionFunctor functor(p, f);
  setOpt(curlpp::options::WriteFunction(functor));
}

inline Connection& HttpConnection::operator=(const Protocol inp) {
  _prot = inp;
  return *this;
}

inline bool HttpConnection::bufEmpty() const { return _buf.empty(); }

inline long HttpConnection::responseCode() {
  return curlpp::infos::ResponseCode::get(_request);
}

inline std::string HttpConnection::httpEffectiveUrl() {
  return curlpp::infos::EffectiveUrl::get(_request);
}

// For curl, a HEADER request is implemented as a GET request with options
//
// inline void Connection::setHeadReq() {
//  setGetReq();
//  _request.setOpt(curlpp::options::Header(true));
//  _request.setOpt(curlpp::options::NoBody(true));
//}
inline void HttpConnection::setPostReq() { setCustomReq("POST"); }

inline void HttpConnection::setDeleteReq() { setCustomReq("DELETE"); }

inline void HttpConnection::setGetReq() { setCustomReq("GET"); }

inline void HttpConnection::setPatchReq() { setCustomReq("PATCH"); }

inline void HttpConnection::setPutReq() { setCustomReq("PUT"); }

// Converts the contents of the default write buffer to a string
//
// This will usually be either Json or an error message
inline const std::string HttpConnection::bufString() const {
  return std::string(_buf.data(), _buf.size());
}

inline void HttpConnection::setErrBuf(char* inp) {
  setOpt(cURLpp::options::ErrorBuffer(inp));
}

inline void HttpConnection::setCustomReq(const std::string inp) {
  setOpt(cURLpp::options::CustomRequest(inp));
}

inline void HttpConnection::setVerbose(const bool inp) {
  setOpt(cURLpp::options::Verbose(inp));
}

inline bool HttpConnection::isError() const {
  return _mode == Mode::LogicError || _mode == Mode::RunError;
}

inline bool HttpConnection::isRunning() const {
  return _mode == Mode::AsyncRun || _mode == Mode::SyncRun;
}

inline void HttpConnection::setHeaderOpts() {
  httpProtocol();
  if (!_headers.empty()) {
    setOpt(curlpp::options::HttpHeader(_headers));
  }
}

template <typename T>
inline void HttpConnection::setOpt(const T& inp) {
  _request.setOpt(inp);
}
}
}

#endif
