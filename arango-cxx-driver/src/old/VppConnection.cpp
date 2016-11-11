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

#include <fuerte/old/HeaderLoader.h>
#include <fuerte/old/VppConnection.h>

namespace arangodb {

namespace dbinterface {

const char VppConnection::ReqName::reqType[] = "reqtype";
const char VppConnection::ReqName::request[] = "request";
const char VppConnection::ReqName::dbName[] = "database";
const char VppConnection::ReqName::url[] = "url";
const char VppConnection::ReqName::port[] = "port";
const char VppConnection::ReqName::parameter[] = "parameter";
const char VppConnection::ReqName::meta[] = "meta";
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

void VppConnection::setHeaderOpts() {
  typedef arangodb::velocypack::Value Value;
  typedef arangodb::velocypack::ValueType ValueType;
  _request.add(ReqName::meta, Value(ValueType::Object));
  for (const ConOption& opt : _headers) {
    _request.add(opt.name(), opt.value());
  }
  _request.close();
}

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

void VppConnection::errorHandler(const std::string& msg) {
  typedef arangodb::velocypack::Value Value;
  typedef arangodb::velocypack::ValueType ValueType;
  {
    Builder b;
    b.add(Value(ValueType::Array));
    b.add(Value(ReqVersion));
    b.add(Value(RespType));
    b.add(Value(0));
    b.close();
    _vpacks[VpRes] = b.steal();
  }
  {
    Builder b;
    b.add(Value(ValueType::Object));
    b.add("errorMessage", Value(msg));
    b.close();
    _vpacks[VpData] = b.steal();
  }
  _mode = Mode::RunError;
}

void VppConnection::errorHandler(const boost::system::error_code& err) {
  errorHandler(err.message());
}

void VppConnection::addToChunks() {
  using arangodb::velocypack::Slice;
  typedef Header::Common::Chunk Chunk;
  for (const VPack vp : _vpacks) {
    if (!vp) {
      continue;
    }
    const uint8_t* ptr = vp->data();
    Slice slice{ptr};
    uint64_t szVPack = slice.byteSize();
    do {
      Chunk& chnk = _chnks.back();
      if (chnk.size() + szVPack <= UINT32_MAX) {
        chnk.insert(chnk.end(), ptr, ptr + szVPack);
        break;
      }
      uint32_t sz = UINT32_MAX - chnk.size();
      chnk.insert(chnk.end(), ptr, ptr + sz);
      ptr += sz;
      szVPack -= sz;
      _chnks.push_back(Chunk{Header::Common::HeaderSize});
    } while (true);
  }
}

void VppConnection::writeMulti() {
  using arangodb::velocypack::Slice;
  typedef Header::Common::Chunk Chunk;
  typedef Header::Common::Chunks Chunks;
  _chnks.resize(1);
  _chnks[0].resize(Header::Multi::HeaderSize);
  addToChunks();
  Chunks::iterator iChnk = _chnks.end();
  Header::Common::SzMsg szMsg = 0;
  Header::Common::ChunkInfo chnkNo = _chnks.size();
  Header::Common::SzChunk szChnk;
  while (--iChnk != _chnks.begin()) {
    szChnk = iChnk->size();
    szMsg += szChnk - Header::Common::HeaderSize;
    Header::Common{_msgId, szChnk, --chnkNo}.headerOut(iChnk->data());
  }
  _buffer.swap(_chnks[0]);
  szChnk = _buffer.size();
  szMsg += szChnk - Header::Multi::HeaderSize;
  chnkNo = _chnks.size();
  Header::Multi{chnkNo, _msgId, szChnk, szMsg}.headerOut(_buffer.data());
  _bufIdx = 0;
  _chnkIdx = 0;
  writeBuffer();
}

void VppConnection::writeSingle() {
  using arangodb::velocypack::Slice;
  _chnks.resize(1);
  _chnks[0].resize(Header::Single::HeaderSize);
  addToChunks();
  _buffer.swap(_chnks[0]);
  Header::Common::SzChunk szChnk = _buffer.size();
  Header::Single{_msgId, szChnk}.headerOut(_buffer.data());
  _bufIdx = 0;
  _chnkIdx = 0;
  writeBuffer();
}

void VppConnection::writeRequest() {
  Header::Common::SzMsg sz = szVPacks();
  if (sz > UINT32_MAX) {
    if (sz > Header::Multi::MaxSzMsg) {
      errorHandler("Message too large");
      return;
    }
    writeMulti();
    return;
  }
  writeSingle();
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
    errorHandler(err);
    return;
  }
  _bufIdx += len;
  if (_bufIdx == _buffer.size()) {
    if (++_chnkIdx == _chnks.size()) {
      getChunks();
      return;
    }
    _buffer.swap(_chnks[_chnkIdx]);
    _bufIdx = 0;
  }
  writeBuffer();
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
  typedef Header::Common::Chunks Chunks;
  typedef Header::Common::SzChunk SzChunk;
  typedef Header::Common::ChunkInfo ChunkInfo;
  typedef Header::Common::SzMsg SzMsg;
  if (err) {
    errorHandler(err);
    return;
  }
  Chunk::const_iterator iData = _buffer.begin();
  const Chunk::const_iterator iEnd = iData + len;
  Chunks::iterator iChnk = _chnks.end() - 1;
  do {
    SzChunk szReq = Header::Common::HeaderSize;
    if (iChnk->size() == Header::Common::HeaderSize) {
      szReq = Header::Common{iChnk->data()}.szChunk();
    }
    if (szReq == iChnk->size()) {
      _chnks.push_back(Chunk{});
      iChnk = _chnks.end() - 1;
      szReq = Header::Common::HeaderSize;
    } else {
      szReq -= iChnk->size();
    }
    len = iEnd - iData;
    if (len < szReq) {
      szReq = len;
    }
    iChnk->insert(iChnk->end(), iData, iData + szReq);
    iData += szReq;
  } while (iData != iEnd);
  do {
    if (iChnk->size() < Header::Common::HeaderSize) {
      break;
    }
    if (Header::Common{iChnk->data()}.szChunk() != iChnk->size()) {
      break;
    }
    if (Header::Common{_chnks[0].data()}.chunkNo() == _chnks.size()) {
      unpackChunks();
      return;
    }
  } while (false);
  getBuffer();
}

void VppConnection::unpackChunks() {
  if (_chnks.empty()) {
    return;
  }
  _vpacks.clear();
  {
    //  Consolidate the chunk data
    Header::Common::Chunks::iterator iChnk = _chnks.begin();
    while (++iChnk != _chnks.end()) {
      Header::Common::Chunk tmp;
      iChnk->swap(tmp);
      _chnks[0].insert(_chnks[0].end(),
                       tmp.begin() + Header::Common::HeaderSize, tmp.end());
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
      vpack->append(&(*iData), szVPack);
      _vpacks.push_back(vpack);
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
    errorHandler(err);
    return;
  }
  // Connection made so try to write
  writeRequest();
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
  _service.reset();

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
  _request.add(ReqName::parameter, Value(ValueType::Object));
  for (const ConOption& opt : _queries) {
    _request.add(opt.name(), opt.value());
  }
  _request.close();
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
  b.add(sVal);
  sVal = sReq.get(ReqName::reqType);
  b.add(sVal);
  sVal = sReq.get(ReqName::request);
  b.add(sVal);
  sVal = sReq.get(ReqName::parameter);
  b.add(sVal);
  sVal = sReq.get(ReqName::meta);
  b.add(sVal);
  b.close();
  _vpacks[VpRes] = b.steal();
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

Connection::VPack VppConnection::result() const { return _vpacks[VpData]; }

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
