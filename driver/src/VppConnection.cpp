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
#include <iostream>

#include "../include/fuerte/HeaderLoader.h"
#include "../include/fuerte/VppConnection.h"

namespace arangodb {

namespace dbinterface {

const char VppConnection::ReqName::reqType[] = "reqtype";
const char VppConnection::ReqName::request[] = "request";
const char VppConnection::ReqName::dbName[] = "database";
const char VppConnection::ReqName::url[] = "url";
const char VppConnection::ReqName::port[] = "port";

const char VppConnection::ReqName::server[] = "server";

Header::Common::MsgId VppConnection::_idKey = std::time(nullptr);

Header::Common::MsgId VppConnection::nextID() {
  static const uint64_t nMagicPrime = 0x00000100000001b3ULL;
  uint64_t nHashVal = 0xcbf29ce484222325ULL;
  const uint8_t* pFirst = reinterpret_cast<const uint8_t*>(&_idKey);
  const uint8_t* const pLast = pFirst + sizeof(uint64_t);

  do {
    nHashVal ^= *pFirst;
    nHashVal *= nMagicPrime;
  } while (++pFirst != pLast);
  _idKey = nHashVal;
  return nHashVal;
}

VppConnection::VppConnection() : Connection(), _service{}, _socket(_service) {
  _buffer.reserve(BufSize);
}

void VppConnection::setHeaderOpts() {}

Header::Common::SzMsg VppConnection::szVPacks() const {
  using arangodb::velocypack::Slice;
  Header::Common::SzMsg sz = 0;
  for (const VPack& vp : _vpacks) {
    if (vp) {
      Slice slice{vp->data()};
      sz += slice.byteSize();
    }
  }
  return sz;
}

void VppConnection::writeSingle() {
  using arangodb::velocypack::Slice;
  Header::Common::SzChunk szChnk = Header::Single::HeaderSize;
  _buffer.resize(Header::Single::HeaderSize);
  for (const VPack vp : _vpacks) {
    if (vp) {
      const uint8_t* ptr = vp->data();
      Slice slice{ptr};
      uint32_t szVPack = slice.byteSize();
      szChnk += szVPack;
      _buffer.insert(_buffer.end(), ptr, ptr + szVPack);
    }
  }
  Header::Single{_msgId, szChnk}.headerOut(_buffer.data());
  _bufIdx = 0;
  writeBuffer();
}

void VppConnection::writeBuffer() {
  auto bndFn = std::bind(&VppConnection::wroteBuffer, this,
                         std::placeholders::_1, std::placeholders::_2);
  auto buf =
      boost::asio::buffer(_buffer.data() + _bufIdx, _buffer.size() - _bufIdx);
  _socket.async_write_some(buf, bndFn);
}

void VppConnection::wroteBuffer(const boost::system::error_code& err,
                                std::size_t len) {
  if (err) {
    //  TODO : handle errors
    _mode = Mode::RunError;
    return;
  }
  _bufIdx += len;
  if (_buffer.size() > _bufIdx) {
    writeBuffer();
    return;
  }
  std::cout << "Buffer out written" << std::endl;
  getChunks();
  //  TODO : get response
}

void VppConnection::getChunks() {
  _chnks.resize(1);
  _chnks[0].clear();
  _buffer.resize(BufSize);
  getBuffer();
}

void VppConnection::getBuffer() {
  auto bndFn = std::bind(&VppConnection::gotBuffer, this, std::placeholders::_1,
                         std::placeholders::_2);
  auto buf = boost::asio::buffer(_buffer.data(), _buffer.size());
  _socket.async_read_some(buf, bndFn);
}

//
// NOTE : Assumes chunks arrive in order
//
void VppConnection::gotBuffer(const boost::system::error_code& err,
                              std::size_t len) {
  typedef Header::Common::Chunk Chunk;
  typedef Header::Common::SzChunk SzChunk;
  typedef Header::Common::ChunkInfo ChunkInfo;
  typedef Header::Common::SzMsg SzMsg;
  if (err) {
    // TODO handle error
    _mode = Mode::RunError;
    return;
  }
  Chunk::const_iterator iData = _buffer.begin();
  do {
    Chunk& chnk = _chnks.back();
    SzChunk szReq = Header::Common::HeaderSize;
    Header::Common header;
    if (chnk.size() >= Header::Common::HeaderSize) {
      header.headerIn(chnk.data());
      szReq = header.szChunk();
    }
    if (len + chnk.size() < szReq) {
      chnk.insert(chnk.end(), iData, iData + len);
      break;
    }
    szReq -= chnk.size();
    chnk.insert(chnk.end(), iData, iData + szReq);
    iData += szReq;
    len -= szReq;
    if (chnk.size() == Header::Common::HeaderSize) {
      continue;
    }
    szReq = header.szChunk();
    if (chnk.size() == szReq) {
      header.headerIn(_chnks[0].data());
      if (_chnks.size() == header.chunkNo()) {
        std::cout << "Chunks received" << std::endl;
        //  Got all the chunks
        unpackChunks();
        _mode = Mode::Done;
        return;
      }
      _chnks.push_back(Chunk{});
    }
  } while (len);
  getBuffer();
}

void VppConnection::unpackChunks() {
  if (_chnks.empty()) {
    return;
  }
  _vpacks.clear();
  {
    //  Consolidate the chunk data
    Header::Common::Chunks::const_iterator iChnk = _chnks.begin();
    while (++iChnk != _chnks.end()) {
      _chnks[0].insert(_chnks[0].end(),
                       iChnk->begin() + Header::Common::HeaderSize,
                       iChnk->end());
    }
    _chnks.resize(1);
  }
  {
    //  Extract the VPacks
    Header::Common::ChunkInfo info = Header::Common::chnkInfo(_chnks[0].data());
    Header::Common::Chunk::const_iterator iData = _chnks[0].begin();
    if (Header::Common::bSingleChunk(info)) {
      iData += Header::Single::HeaderSize;
    } else {
      iData += Header::Multi::HeaderSize;
    }
    while (iData != _chnks[0].end()) {
      VPack vpack{new VBuffer{}};
      arangodb::velocypack::Slice slice{&(*iData)};
      size_t szVPack = slice.byteSize();
      vpack->append(reinterpret_cast<const char*>(&(*iData)), szVPack);
      _vpacks.push_back(vpack);
      std::cout << json(vpack) << std::endl;
      iData += szVPack;
    }
  }
}

void VppConnection::getConnection() {
  typedef arangodb::velocypack::Builder Builder;
  using arangodb::velocypack::Slice;
  using arangodb::velocypack::Value;
  using arangodb::velocypack::ValueType;
  using arangodb::velocypack::ValueLength;
  typedef boost::asio::ip::tcp::resolver Resolver;
  Slice sReq{_request.start()};
  Slice sVal{sReq.get(ReqName::server)};
  std::string url = sVal.copyString();
  std::string port;
  Resolver resolver(_service);
  sVal = sReq.get(ReqName::port);
  port = sVal.copyString();
  Resolver::query query(url, port);
  Resolver::iterator endpoint_iterator = resolver.resolve(query);
  auto bndFn = std::bind(&VppConnection::gotConnection, this,
                         std::placeholders::_1, endpoint_iterator);
  _socket.async_connect(*endpoint_iterator, bndFn);
}

void VppConnection::gotConnection(const boost::system::error_code& err,
                                  tcp::resolver::iterator endpoint_iterator) {
  if (err) {
    _socket.close();
    if (++endpoint_iterator != tcp::resolver::iterator()) {
      // The connection failed. Try the next endpoint in the list.
      auto bndFn = std::bind(&VppConnection::gotConnection, this,
                             std::placeholders::_1, endpoint_iterator);
      _socket.async_connect(*endpoint_iterator, bndFn);
      return;
    }
    //  TODO : Report the error
    _mode = Mode::RunError;
    return;
  }
  std::cout << "Connection made" << std::endl;
  // Connection made so try to write
  writeSingle();
}

//
//
//
//
Connection& VppConnection::reset() {
  namespace velocypack = arangodb::velocypack;
  using velocypack::ValueType;
  using velocypack::Value;

  if (_socket.is_open()) {
    _socket.cancel();
    _socket.close();
  }
  _mode = Mode::SyncRun;
  _msgId = nextID();
  _queries.clear();
  _headers.clear();
  _vpacks.resize(2);
  _vpacks[0] = nullptr;
  _vpacks[1] = nullptr;
  _request.clear();
  _request.add(Value(ValueType::Object));

  return *this;
}

void VppConnection::setUrl(const Url& inp) {
  namespace velocypack = arangodb::velocypack;
  using velocypack::ValueType;
  using velocypack::Value;
  std::string url = inp.serverUrl();
  std::string port{"80"};
  std::string::size_type len = url.find("//");
  _request.add(ReqName::dbName, Value(inp.dbName()));
  if (len != std::string::npos) {
    url = url.substr(len + 2);
  }
  len = url.find(':');
  if (len != std::string::npos) {
    port = url.substr(len + 1);
    url = url.substr(0, len);
  }
  _request.add(ReqName::port, Value(port));
  _request.add(ReqName::server, Value(url));
  url = inp.tailUrl();
  _request.add(ReqName::request, Value(url));
}

//
//  Used to set the Parameter, (REST Queries),  and meta options,  (REST
//  Headers)
//
void VppConnection::setReqOpts(Builder& b, const ConOption::vector& opts) {
  namespace velocypack = arangodb::velocypack;
  using velocypack::ValueType;
  using velocypack::Value;

  b.add(Value(ValueType::Object));
  for (const ConOption& opt : opts) {
    b.add(opt.name(), opt.value());
  }
  b.close();
}

void VppConnection::addHeader(const ConOption& inp) { _headers.push_back(inp); }

void VppConnection::addQuery(const ConOption& inp) { _queries.push_back(inp); }

void VppConnection::addHeader(ConOption&& inp) { _headers.push_back(inp); }

void VppConnection::addQuery(ConOption&& inp) { _queries.push_back(inp); }

void VppConnection::setPostReq() {
  namespace velocypack = arangodb::velocypack;
  using velocypack::Value;
  _request.add(ReqName::reqType, Value(RequestValue::ReqPost));
}

void VppConnection::setDeleteReq() {
  namespace velocypack = arangodb::velocypack;
  using velocypack::Value;
  _request.add(ReqName::reqType, Value(RequestValue::ReqDelete));
}

void VppConnection::setGetReq() {
  namespace velocypack = arangodb::velocypack;
  using velocypack::Value;
  _request.add(ReqName::reqType, Value(RequestValue::ReqGet));
}

void VppConnection::setPutReq() {
  namespace velocypack = arangodb::velocypack;
  using velocypack::Value;
  _request.add(ReqName::reqType, Value(RequestValue::ReqPut));
}

void VppConnection::setPatchReq() {
  namespace velocypack = arangodb::velocypack;
  using velocypack::Value;
  _request.add(ReqName::reqType, Value(RequestValue::ReqPatch));
}

void VppConnection::setPostField(const std::string& inp) {
  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&inp.front());
  std::string::size_type sz = inp.size();
  _vpacks[VpData] = vpack(ptr, sz);
}

void VppConnection::setPostField(const VPack data) { _vpacks[VpData] = data; }

//
//  Create the request VPack
//
//  Converts a VPack object to the required VPack array
//
void VppConnection::reqVPack() {
  typedef arangodb::velocypack::Builder Builder;
  using arangodb::velocypack::Slice;
  using arangodb::velocypack::Value;
  using arangodb::velocypack::ValueType;
  using arangodb::velocypack::ValueLength;
  Builder b;
  Slice sReq{_request.start()};
  Slice sVal{sReq.get(ReqName::dbName)};
  b.add(Value(ValueType::Array));
  b.add(Value{RequestValue::ReqVersion});
  b.add(Value{RequestValue::ReqType});
  b.add(Value(sVal.copyString()));
  sVal = sReq.get(ReqName::reqType);
  b.add(Value{sVal.getInt()});
  sVal = sReq.get(ReqName::request);
  b.add(Value(sVal.copyString()));
  setReqOpts(b, _queries);
  setReqOpts(b, _headers);
  b.close();
  _vpacks[VpRes] = b.steal();
  std::cout << json(_vpacks[0]) << std::endl;
}

void VppConnection::setBuffer() {
  _request.close();
  reqVPack();
  getConnection();
}

void VppConnection::setAsynchronous(const bool bAsync) {
  _mode = bAsync ? Mode::AsyncRun : Mode::SyncRun;
}

void VppConnection::run() {
  switch (_mode) {
    case Mode::AsyncRun: {
      _service.run_one();
      if (_service.stopped()) {
        _mode = Mode::Done;
      }
      break;
    }

    case Mode::SyncRun: {
      while (!_service.stopped()) {
        _service.run();
      }
      _mode = Mode::Done;
      break;
    }

    default:;
  }
}

bool VppConnection::isRunning() const { return _mode == Mode::AsyncRun; }

Connection::VPack VppConnection::result() const { return _vpacks[1]; }

long VppConnection::responseCode() {
  typedef arangodb::velocypack::Builder Builder;
  using arangodb::velocypack::Slice;
  using arangodb::velocypack::Value;
  using arangodb::velocypack::ValueType;
  Slice sRes{_vpacks[VpRes]->data()};
  Slice sObj{sRes[VpResIdx]};
  return sObj.getInt();
}
}
}
