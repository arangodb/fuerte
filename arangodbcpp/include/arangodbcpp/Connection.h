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

#ifndef FUERTE_CONNECTION_H
#define FUERTE_CONNECTION_H 1

#include <vector>
#include <list>
#include <curlpp/Easy.hpp>
#include <curlpp/Multi.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

#include "ConnectionBase.h"

namespace arangodb {

namespace dbinterface {

//
//  Provide network connection to Arango database
//  Hide the underlying use of cURL for implementation
//
class Connection : public ConnectionBase {
 public:
  typedef std::shared_ptr<Connection> SPtr;
  typedef std::list<std::string> HeaderList;
  typedef std::string QueryList;
  Connection();
  virtual ~Connection();
  ConnectionBase& operator=(const Protocol in);
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
  ConnectionBase& reset();
  template <typename T>
  void setBuffer(T* p, size_t (T::*f)(char* p, size_t sz, size_t m));
  void setBuffer(size_t (*f)(char* p, size_t sz, size_t m));
  void setBuffer();
  long responseCode();
  std::string httpEffectiveUrl();

  const std::string bufString() const;
  VPack result(const bool bSort) const;
  VPack fromVPData() const;
  VPack fromJSon(const bool bSorted = true) const;
  void setAsynchronous(const bool bAsync = false);
  void run();
  void runAgain(bool bAsync);
  bool isError() const;
  bool isRunning() const;
  bool bufEmpty() const;

  static std::string json(const VPack& v, bool bSort = false);
  static VPack vpack(const uint8_t* data, std::size_t sz, bool bSort);

 private:
  enum class Mode : uint8_t {
    Clear = 0,
    AsyncRun = 1,
    SyncRun = 2,
    Done = 3,
    LogicError = 4,
    RunError = 5
  };
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
inline void Connection::setBuffer(T* p, size_t (T::*f)(char*, size_t, size_t)) {
  curlpp::types::WriteFunctionFunctor functor(p, f);
  setOpt(curlpp::options::WriteFunction(functor));
}

inline ConnectionBase& Connection::operator=(const Protocol inp) {
  _prot = inp;
  return *this;
}

inline bool Connection::bufEmpty() const { return _buf.empty(); }

inline long Connection::responseCode() {
  return curlpp::infos::ResponseCode::get(_request);
}

inline std::string Connection::httpEffectiveUrl() {
  return curlpp::infos::EffectiveUrl::get(_request);
}

//
// For curl, a HEADER request is implemented as a GET request with options
//
// inline void Connection::setHeadReq() {
//  setGetReq();
//  _request.setOpt(curlpp::options::Header(true));
//  _request.setOpt(curlpp::options::NoBody(true));
//}

inline void Connection::setPostReq() { setCustomReq("POST"); }

inline void Connection::setDeleteReq() { setCustomReq("DELETE"); }

inline void Connection::setGetReq() { setCustomReq("GET"); }

inline void Connection::setPatchReq() { setCustomReq("PATCH"); }

inline void Connection::setPutReq() { setCustomReq("PUT"); }

// Converts the contents of the default write buffer to a string
//
// This will usually be either JSon or an error message

inline const std::string Connection::bufString() const {
  return std::string(_buf.data(), _buf.size());
}

inline void Connection::setErrBuf(char* inp) {
  setOpt(cURLpp::options::ErrorBuffer(inp));
}

inline void Connection::setCustomReq(const std::string inp) {
  setOpt(cURLpp::options::CustomRequest(inp));
}

inline void Connection::setVerbose(const bool inp) {
  setOpt(cURLpp::options::Verbose(inp));
}

inline bool Connection::isError() const {
  return _mode == Mode::LogicError || _mode == Mode::RunError;
}

inline bool Connection::isRunning() const {
  return _mode == Mode::AsyncRun || _mode == Mode::SyncRun;
}

inline void Connection::setHeaderOpts() {
  httpProtocol();
  if (!_headers.empty()) {
    setOpt(curlpp::options::HttpHeader(_headers));
  }
}

template <typename T>
inline void Connection::setOpt(const T& inp) {
  _request.setOpt(inp);
}
}
}

#endif
