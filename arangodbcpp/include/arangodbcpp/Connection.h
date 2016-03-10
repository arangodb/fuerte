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
#ifndef FUERTE_CONNECTION_H
#define FUERTE_CONNECTION_H

#include <memory>
#include <vector>
#include <list>
#include <velocypack/Buffer.h>
#include <curlpp/Easy.hpp>
#include <curlpp/Multi.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

namespace arangodb {

namespace dbinterface {

//
//  Provide network connection to Arango database
//  Hide the underlying use of cURL for implementation
//
class Connection {
 public:
  enum class QueryPrefix : uint8_t { First = '?', Next = '&' };
  typedef std::shared_ptr<Connection> SPtr;
  typedef arangodb::velocypack::Buffer<uint8_t> VBuffer;
  typedef std::shared_ptr<VBuffer> VPack;
  typedef std::list<std::string> HttpHeaderList;
  typedef char ErrBuf[CURL_ERROR_SIZE];
  Connection();
  ~Connection();
  void setUrl(const std::string& inp);
  void setUrl(std::string&& inp);
  void setErrBuf(char* inp);
  void setPostField(const std::string& inp);
  void setPostField(const VPack data);
  void setJsonContent(HttpHeaderList& headers);
  void setHeaderOpts(HttpHeaderList& inp);
  void setCustomReq(const std::string inp);
  void setPostReq();
  void setDeleteReq();
  void setHeadReq();
  void setGetReq();
  void setPutReq();
  void setPatchReq();
  void setVerbose(bool inp);
  Connection& reset();
  template <typename T>
  void setBuffer(T* p, size_t (T::*f)(char* p, size_t sz, size_t m));
  void setBuffer(size_t (*f)(char* p, size_t sz, size_t m));
  void setBuffer();
  long httpResponseCode();

  const std::string bufString() const;
  VPack fromJSon(bool bSorted = true) const;
  VPack notProcessed() const;
  VPack noHost() const;
  void setSync(bool bAsync = false);
  void run();
  bool isError() const;
  bool isRunning() const;
  bool bufEmpty() const;

  static std::string json(const VPack& v, bool bSort = false);
  static std::string strValue(const VPack res, std::string attrib);

 private:
  enum : uint8_t {
    F_Clear = 0,
    F_Multi = 1,
    F_Running = 2,
    F_Done = 4,
    F_LogicError = 8,
    F_RunError = 16
  };
  typedef std::vector<char> ChrBuf;
  size_t WriteMemoryCallback(char* ptr, size_t size, size_t nmemb);
  template <typename T>
  void setOpt(const T& inp);
  void asyncRun();
  void syncRun();
  void errFound(const std::string& inp, uint8_t err = F_RunError);
  void httpResponse();
  void setBuffer(const std::string& inp);

  curlpp::Easy _request;
  curlpp::Multi _async;
  ChrBuf _buf;
  uint8_t _flgs;
};

template <typename T>
inline void Connection::setBuffer(T* p, size_t (T::*f)(char*, size_t, size_t)) {
  curlpp::types::WriteFunctionFunctor functor(p, f);
  setOpt(curlpp::options::WriteFunction(functor));
}

inline bool Connection::bufEmpty() const { return _buf.empty(); }

inline std::string operator+(const Connection::QueryPrefix pre,
                             const std::string& query) {
  return static_cast<char>(pre) + query;
}

inline std::string operator+(const Connection::QueryPrefix pre,
                             std::string&& query) {
  return static_cast<char>(pre) + query;
}

inline long Connection::httpResponseCode() {
  return curlpp::infos::ResponseCode::get(_request);
}

//
// For curl, a HEADER request is implemented as a GET request with options
//
inline void Connection::setHeadReq() {
  setGetReq();
  _request.setOpt(curlpp::options::Header(true));
  _request.setOpt(curlpp::options::NoBody(true));
}

inline void Connection::setPostReq() { setCustomReq("POST"); }

inline void Connection::setDeleteReq() { setCustomReq("DELETE"); }

inline void Connection::setGetReq() { setCustomReq("GET"); }

inline void Connection::setPatchReq() { setCustomReq("PATCH"); }

inline void Connection::setPutReq() { setCustomReq("PUT"); }

//
// Converts the contents of the default write buffer to a string
//
// This will usually be either JSon or an error message
//
inline const std::string Connection::bufString() const {
  return std::string(_buf.data(), _buf.size());
}

inline void Connection::setBuffer(const std::string& inp) {
  size_t len = inp.length();
  _buf.resize(len);
  memcpy(_buf.data(), inp.data(), len);
}

//
// Flags an error has occured and transfers the error message
// to the default write buffer
//
inline void Connection::errFound(const std::string& inp, uint8_t err) {
  std::string json{"{ \"errorMessage\":\"" + inp + "\"}"};
  setBuffer(json);
  _flgs |= err;
}

inline void Connection::setErrBuf(char* inp) {
  setOpt(cURLpp::options::ErrorBuffer(inp));
}

inline void Connection::setCustomReq(const std::string inp) {
  setOpt(cURLpp::options::CustomRequest(inp));
}

inline void Connection::setVerbose(bool inp) {
  setOpt(cURLpp::options::Verbose(inp));
}

inline bool Connection::isError() const {
  return (_flgs & (F_LogicError | F_RunError)) != 0;
}

inline bool Connection::isRunning() const { return (_flgs & F_Running) != 0; }

inline void Connection::setHeaderOpts(HttpHeaderList& inp) {
  if (!inp.empty()) {
    setOpt(curlpp::options::HttpHeader(inp));
  }
}

inline void Connection::setUrl(const std::string& inp) {
  setOpt(curlpp::options::Url(inp));
}

inline void Connection::setUrl(std::string&& inp) {
  setOpt(curlpp::options::Url(inp));
}

template <typename T>
inline void Connection::setOpt(const T& inp) {
  _request.setOpt(inp);
}
}
}

#endif  // FUERTE_CONNECTION_H
