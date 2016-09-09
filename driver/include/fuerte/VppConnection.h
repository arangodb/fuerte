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

#ifndef VPPCONNECTION_H

#define VPPCONNECTION_H

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>
#include <vector>

#include "Connection.h"
#include "HeaderCommon.h"

namespace arangodb {

namespace dbinterface {

//
//  Provide network connection to Arango database using VelocyStream
//  Hides the underlying use of Boost ASIO for implementation
//
class VppConnection : public Connection {
 private:
  typedef boost::asio::io_service Service;
  typedef std::shared_ptr<Service> ServicePtr;
  typedef boost::asio::ip::tcp tcp;
  typedef ConOption::vector HeaderList;
  typedef ConOption::vector QueryList;

 public:
  typedef std::shared_ptr<VppConnection> SPtr;
  VppConnection();
  Connection& operator=(const Protocol in) override;
  Connection& reset() override;
  void defaultContentType(Format inp) override;
  void defaultAccept(Format inp) override;
  void setUrl(const Url& inp) override;
  void setHeaderOpts() override;
  void addHeader(const ConOption& inp) override;
  void addQuery(const ConOption& inp) override;
  void addHeader(ConOption&& inp) override;
  void addQuery(ConOption&& inp) override;
  void setPostReq() override;
  void setDeleteReq() override;
  void setGetReq() override;
  void setPutReq() override;
  void setPatchReq() override;
  void setPostField(const std::string& inp) override;
  void setPostField(const VPack data) override;
  void setBuffer() override;
  void setAsynchronous(const bool bAsync = false) override;
  void run() override;
  bool isRunning() const override;
  VPack result() const override;
  long responseCode() override;

 private:
  struct ReqName {
    static const char reqType[];
    static const char request[];
    static const char dbName[];
    static const char url[];
    static const char port[];
    static const char server[];
  };
  enum VpIdx : uint16_t { VpRes = 0, VpData = 1, VpResIdx = 2 };
  enum : uint16_t { BufSize = 10000 };
  enum RequestValue : int16_t {
    ReqVersion = 1,
    ReqType = 1,
    ReqDelete = 0,
    ReqGet = 1,
    ReqPost = 2,
    ReqPut = 3,
    ReqPatch = 5
  };
  static void setReqOpts(Builder& b, const ConOption::vector& opts);
  void setReqType();
  void reqVPack();
  Header::Common::MsgId nextID();
  Header::Common::SzMsg szVPacks() const;
  void writeSingle();
  void getConnection();
  void gotConnection(const boost::system::error_code& err,
                     tcp::resolver::iterator endpoint_iterator);
  void unpackChunks();
  void writeBuffer();
  void wroteBuffer(const boost::system::error_code& err, std::size_t len);
  void getChunks();
  void getBuffer();
  void gotBuffer(const boost::system::error_code& err, std::size_t len);

  static Header::Common::MsgId _idKey;

  Service _service;
  tcp::socket _socket;

  Header::Common::MsgId _msgId;

  //  Connection details
  Builder _request;
  HeaderList _headers;
  QueryList _queries;

  Mode _mode;

  Header::Common::Chunks _chnks;
  Header::Common::Chunk _buffer;
  Header::Common::Chunk::size_type _bufIdx;
  VPacks _vpacks;
};

inline Connection& VppConnection::operator=(const Protocol) { return *this; };

inline void VppConnection::defaultContentType(Format){};

inline void VppConnection::defaultAccept(Format){};
}
}

#endif  // VPPCONNECTION_H
