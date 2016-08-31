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

#include <fuerte/VppConnection.h>

namespace arangodb {

namespace dbinterface {

namespace Header
{

class Common
{
public:
    typedef VppConnection::HeaderType::MsgId MsgId;
    typedef VppConnection::HeaderType::SzMsg SzMsg;
    typedef VppConnection::HeaderType::SzChunk SzChunk;
    typedef VppConnection::HeaderType::ChunkInfo ChunkInfo;
    typedef VppConnection::Chunk Chunk;
    Common();
    Common(const uint8_t *ptr);
    Common(MsgId id, SzChunk sz);
    Common(ChunkInfo nChks, MsgId id, SzChunk sz);
    Common(MsgId id, SzChunk sz, ChunkInfo chnkNo );
    virtual void headerOut(uint8_t *ptr) const; 
    SzChunk szChunk() const;
    virtual bool bFirstChunk() const;
    virtual bool bSingleChunk() const;
    virtual ChunkInfo chunkNo() const;
    virtual ChunkInfo noChunks() const;
    virtual SzChunk szHeader() const;
    virtual SzMsg szChunks() const;
    
    static ChunkInfo chnkInfo(const uint8_t *ptr);
    static bool bFirstChunk(ChunkInfo info);
    static bool bSingleChunk(ChunkInfo info);

protected:
    enum : SzChunk { 
      HeaderSize = 16
      , SingleChunk = 3
      , IdxSzChunk = 0
      , IdxChunkInfo = sizeof(SzChunk)
      , IdxMsgId = IdxChunkInfo + sizeof(ChunkInfo)
      , IdxMsgSize = IdxMsgId + sizeof(MsgId)
    };
private:
    SzChunk _szChnk;
    ChunkInfo _chnkNo;
    MsgId _msgId;
};

inline Common::Common()
{
}

Common::Common(const uint8_t *ptr)
{
  _szChnk = *reinterpret_cast<const SzChunk *>(ptr + IdxSzChunk);
  _chnkNo = *reinterpret_cast<const SzChunk *>(ptr + IdxChunkInfo);
  _msgId = *reinterpret_cast<const SzMsg *>(ptr + IdxMsgId);
}

// 
//  Constructor for header a single chunk message
// 
Common::Common(MsgId id, SzChunk sz)
  :  _szChnk(sz), _chnkNo(SingleChunk), _msgId(id)
{
}

// 
//  Constructor for header for a multiple chunk message
// 
Common::Common(ChunkInfo nChks, MsgId id, SzChunk sz)
  :  _szChnk(sz), _chnkNo(1 + (nChks <<  1)), _msgId(id)
{
}

// 
//  Constructor for header for an extra chunk
// 
Common::Common(MsgId id, SzChunk sz, ChunkInfo chnkNo)
  :  _szChnk(sz), _chnkNo(chnkNo <<  1), _msgId(id)
{
}

inline Common::ChunkInfo Common::chnkInfo(const uint8_t *ptr)
{
  return *reinterpret_cast<const ChunkInfo *>(ptr + IdxChunkInfo);
}

void Common::headerOut(uint8_t *ptr) const
{
  *reinterpret_cast<SzChunk *>(ptr + IdxSzChunk) = _szChnk;
  *reinterpret_cast<SzChunk *>(ptr + IdxSzChunk) = _chnkNo;
  *reinterpret_cast<SzMsg *>(ptr + IdxMsgId) = _msgId;
}

inline Common::SzChunk Header::Common::szChunk() const
{
    return _szChnk;
}

inline Common::ChunkInfo Header::Common::noChunks() const
{
    return 0;
}

inline bool Common::bFirstChunk(ChunkInfo info)
{
    return (info & 0x1) != 0;
}

inline bool Common::bFirstChunk() const
{
    return bFirstChunk(_chnkNo);
}

inline bool Common::bSingleChunk(ChunkInfo info)
{
    return info == SingleChunk;
}

inline bool Common::bSingleChunk() const
{
    return bSingleChunk(_chnkNo) ;
}

inline Common::ChunkInfo Header::Common::chunkNo() const
{
  return _chnkNo >> 1;
}

Common::SzChunk Common::szHeader() const
{
    return HeaderSize;
}

class Single : public Common
{
public:
    Single();
    Single(const uint8_t *ptr);
    Single(MsgId id,  SzChunk szChnk);
    SzChunk szHeader() const override;
    ChunkInfo noChunks() const override;
    ChunkInfo chunkNo() const override;

private:
    enum :  SzChunk { HeaderSize = 16 };
};

inline Single::Single() : Common{}
{
}

inline Single::Single(const uint8_t *ptr) : Common{ ptr }
{
}

Single::Single(MsgId id,  SzChunk szChnk) : Common{ id, szChnk }
{
}

inline Common::ChunkInfo Single::chunkNo() const
{
  return 0;
}

inline Common::SzChunk Single::szHeader() const
{
    return HeaderSize;
}

inline Common::ChunkInfo Single::noChunks() const
{
    return 1;
}

class Multi : public Common
{
public:
  Multi();
  Multi(const uint8_t *ptr);
  Multi(ChunkInfo noChunks, MsgId msgId, SzChunk szChunk, SzMsg szMsg);
  SzChunk szHeader() const override;
  SzMsg szChunks() const override;
  bool bFirstChunk() const override;
  bool bSingleChunk() const override;
  ChunkInfo chunkNo() const override;
  void headerOut(uint8_t *ptr) const override;

protected:
  enum : SzChunk { HeaderSize = 24 };
private:
  MsgId _szMsg;
};

inline Multi::Multi() : Common{}
{
}

 Multi::Multi(ChunkInfo noChunks, MsgId msgId, SzChunk szChunk, SzMsg szMsg)
   :  Common(noChunks, msgId, szChunk),  _szMsg(szMsg)
{
}

Multi::Multi(const uint8_t *ptr) : Common(ptr)
{
  _szMsg = *reinterpret_cast<const MsgId *>(ptr + IdxMsgSize);
}

void Multi::headerOut(uint8_t *ptr) const
{
  Common::headerOut(ptr);
  ptr += Common::HeaderSize;
  *reinterpret_cast<MsgId *>(ptr) = _szMsg;
}

inline Common::ChunkInfo Multi::chunkNo() const
{
  return 0;
}

inline bool Multi::bFirstChunk() const
{
  return true;
}

inline bool Multi::bSingleChunk() const
{
  return false;
}

inline Common::SzChunk Multi::szHeader() const
{
    return HeaderSize;
}

Common::SzMsg Multi::szChunks() const
{
  MsgId res = noChunks() - 1;
  res *= Common::HeaderSize;
  res +=  HeaderSize;
  return res;
}

class Loader
{
  union 
  {
    uint8_t _common[sizeof(Common)];
    uint8_t _single[sizeof(Single)];
    uint8_t _multi[sizeof(Multi)];
  } _hdr;
  
public:
  Common &operator()(VppConnection::Chunk &chnk);
};

Common &Loader::operator()(VppConnection::Chunk &chnk)
{
  uint8_t *ptr = chnk.data();
  Common::ChunkInfo info{ Common::chnkInfo(ptr) };
  Common *pHeader;
  if (Common::bSingleChunk(info))
  {
    pHeader = new (_hdr._single) Single{ ptr };
    return *pHeader;
  }
  if (Common::bFirstChunk(info))
  {
    pHeader = new (_hdr._multi) Multi{ ptr };
    return *pHeader;
  }
  pHeader = new (_hdr._common) Common{ptr};
  return *pHeader;
}

}

namespace ReqName
{

char request[] = "request";
char reqtype[] = "reqtype";
char dbName[] = "database";
char url[] = "url";
char port[] = "port";
char server[] = "server";

};

enum Request : int16_t {
  Type = 1
  , Version = 1
  , Delete = 0
  , Get = 1
  , Post = 2
  , Put = 3
  , Patch = 5
};

VppConnection::HeaderType::MsgId  VppConnection::_idKey = std::time(nullptr);


VppConnection::HeaderType::MsgId VppConnection::nextID()
{
  static const uint64_t nMagicPrime = 0x00000100000001b3ULL;
  uint64_t nHashVal = 0xcbf29ce484222325ULL;
  const uint8_t *pFirst = reinterpret_cast<const uint8_t *> ( &_idKey );
  const uint8_t *const pLast = pFirst + sizeof ( uint64_t );

  do
  {
    nHashVal ^= *pFirst;
    nHashVal *= nMagicPrime;
  }
  while ( ++pFirst != pLast );
  _idKey = nHashVal;
  return nHashVal;
}

VppConnection::VppConnection() : Connection()
    ,_service{}, _socket(_service)
{
}

void VppConnection::gotConnection(const boost::system::error_code &err,
                     tcp::resolver::iterator endpoint_iterator)
{
  if (err) 
  {
    _socket.close();
    if (++endpoint_iterator != tcp::resolver::iterator()) 
    {
        // The connection failed. Try the next endpoint in the list.
        auto bndFn = std::bind(&VppConnection::gotConnection, this,
                            std::placeholders::_1, endpoint_iterator);
        _socket.async_connect(*endpoint_iterator, bndFn);
    }
  }
}

Connection& VppConnection::reset() 
{
    namespace velocypack = arangodb::velocypack;
    using velocypack::ValueType;
    using velocypack::Value;

    _request.clear();
    _request.add(Value(ValueType::Object));
    
    return *this;
}

void VppConnection::setUrl(const Url& inp)
{
  namespace velocypack = arangodb::velocypack;
  using velocypack::ValueType;
  using velocypack::Value;
  std:: string url = inp.serverUrl();
  std:: string port { "80" };
  std::string::size_type len = url.find("//");
  _request.add(ReqName::dbName, Value(inp.dbName()));    
  _request.add(ReqName::request, Value(inp.tailUrl()));
  if (len !=  std::string::npos)
  {
    url = url.substr(len + 2);
  }
  len =url.find(':');
  if (len !=  std::string::npos)
  {
    port = url.substr(len +1);
    url = url.substr(0, len);
  }
  _request.add(ReqName::port, Value(port));
  _request.add(ReqName::server, Value(url));
  /*
  if (_socket.is_open())
  {
    _socket.close();
  }
  boost::asio::ip::tcp::resolver resolver(_service);
  boost::asio::ip::tcp::resolver::query query(url, port);
  boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
      resolver.resolve(query);
  auto bndFn = std::bind ( &VppConnection::gotConnection, this
                            , std::placeholders::_1
                            , endpoint_iterator );
  _socket.async_connect ( *endpoint_iterator, bndFn );
  */
}

void VppConnection::setHeaderOpts()
{
    namespace velocypack = arangodb::velocypack;
    using velocypack::ValueType;
    using velocypack::Value;

   _request.add("meta", Value(ValueType::Object));
   for (ConOption &opt : _headers)
   {
       _request.add(opt.name(),opt.value());
   }
   _request.close();
}

void VppConnection::setQueryOpts()
{
    namespace velocypack = arangodb::velocypack;
    using velocypack::ValueType;
    using velocypack::Value;

   _request.add("parameter", Value(ValueType::Object));
   for (ConOption &opt : _queries)
   {
       _request.add(opt.name(),opt.value());
   }
   _request.close();
}

void VppConnection::addHeader(const ConOption& inp)
{
    _headers.push_back(inp);
}

void VppConnection::addQuery(const ConOption& inp) 
{
    _queries.push_back(inp);
}

void VppConnection::addHeader(ConOption&& inp)
{
    _headers.push_back(inp);
}

void VppConnection::addQuery(ConOption&& inp)
{
    _queries.push_back(inp);
}

void VppConnection::setPostReq()
{
  namespace velocypack = arangodb::velocypack;
  using velocypack::Value;
  _request.add(ReqName::reqtype, Value(RequestType::ReqPost));
}

void VppConnection::setDeleteReq()
{
  namespace velocypack = arangodb::velocypack;
  using velocypack::Value;
  _request.add(ReqName::reqtype, Value(Request::Delete));
}

void VppConnection::setGetReq()
{
  namespace velocypack = arangodb::velocypack;
  using velocypack::Value;
  _request.add(ReqName::reqtype, Value(RequestType::ReqGet));
}

void VppConnection::setPutReq()
{
  namespace velocypack = arangodb::velocypack;
  using velocypack::Value;
  _request.add(ReqName::reqtype, Value(RequestType::ReqPut));
}

void VppConnection::setPatchReq()
{
  namespace velocypack = arangodb::velocypack;
  using velocypack::Value;
  _request.add(ReqName::reqtype, Value(RequestType::ReqPatch));
}

void VppConnection::setPostField(const std::string& inp)
{
}

void VppConnection::setPostField(const VPack data) 
{
}

void VppConnection::setBuffer()
{
  using arangodb::velocypack::Builder;
  using arangodb::velocypack::Slice;
  using arangodb::velocypack::Value;
  using arangodb::velocypack::ValueType;
  using arangodb::velocypack::ValueLength;
  using ReqName::request;
  using ReqName::reqtype;
  auto getString = [](Slice &s) ->  std::string
  {
    ValueLength len;
    const char *str = s.getString(len);
    return std::string{str, len};
  };
  Builder b{_request};
  Slice sObj{b.data()};
  b.close();
  _request.clear();
  
}

void VppConnection::setAsynchronous(const bool bAsync)
{
}

void VppConnection::run()
{
}

bool VppConnection::isRunning() const
{
    return true;
}

Connection::VPack VppConnection::result() const
{
  return Connection::VPack{};
}

long VppConnection::responseCode()
{
    return 0;
}

}
}

