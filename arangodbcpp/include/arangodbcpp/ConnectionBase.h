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

#ifndef FUERTE_CONNECTIONBASE
#define FUERTE_CONNECTIONBASE

#include <memory>

#include <velocypack/Buffer.h>
#include <velocypack/Builder.h>

#include "ConOption.h"
#include "arangodbcpp/ConnectionUrl.h"

namespace arangodb {

namespace dbinterface {

//
// Defines the interfaces required for a connection
//
class Connection {
 public:
  typedef std::shared_ptr<Connection> SPtr;
  typedef arangodb::velocypack::Buffer<uint8_t> VBuffer;
  typedef arangodb::velocypack::Builder Builder;
  typedef std::shared_ptr<VBuffer> VPack;
#ifdef FUERTE_CONNECTIONURL
  typedef ConnectionUrl Url;
#else
  typedef std::string Url;
#endif
  enum class Protocol : uint8_t {
    JSon = 0,   // JSon <=> JSon
    VPackJSon,  // VPack <=> Json
    JSonVPack,  // JSon <=> VPack
    VPack,      // VPack <=> Vpack
    VStream     // VelocyStream
  };
  enum class Format { JSon, VPack };

  virtual ~Connection();

  virtual Connection& operator=(const Protocol in) = 0;
  virtual Connection& reset() = 0;
  virtual void defaultContentType(Format inp) = 0;
  virtual void defaultAccept(Format inp) = 0;
  virtual void setUrl(const Url& inp) = 0;
  virtual void setHeaderOpts() = 0;
  virtual void addHeader(const ConOption& inp) = 0;
  virtual void addQuery(const ConOption& inp) = 0;
  virtual void addHeader(ConOption&& inp) = 0;
  virtual void addQuery(ConOption&& inp) = 0;
  virtual void setPostReq() = 0;
  virtual void setDeleteReq() = 0;
  virtual void setGetReq() = 0;
  virtual void setPutReq() = 0;
  virtual void setPatchReq() = 0;
  virtual void setPostField(const std::string& inp) = 0;
  virtual void setPostField(const VPack data) = 0;
  virtual void setBuffer() = 0;
  virtual void setAsynchronous(const bool bAsync = false) = 0;
  virtual void run() = 0;
  virtual bool isRunning() const = 0;
  virtual VPack result(const bool bSort) const = 0;
  virtual long responseCode() = 0;
};

inline Connection::~Connection() {}
}
}

#endif
