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

#include <vector>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

#include <fuerte/Connection.h>

namespace arangodb {

namespace dbinterface {


//
//  Provide network connection to Arango database using VelocyStream
//  Hide the underlying use of Boost ASIO for implementation
//
class VppConnection : public Connection
{
public:
    struct HeaderType 
    {
        typedef uint64_t MsgId;
        typedef uint64_t SzMsg;
        typedef uint32_t SzChunk;
        typedef uint16_t ChunkInfo;
    };
    typedef boost::asio::io_service Service;
    typedef std::shared_ptr<Service> ServicePtr;
    typedef boost::asio::ip::tcp tcp;
    typedef ConOption::vector HeaderList;
    typedef ConOption::vector QueryList;
    typedef std::vector<uint8_t> Chunk;
    typedef std::vector<Chunk> Chunks;

    VppConnection();
    Connection& operator=(const Protocol in);
    Connection& reset() ;
    void defaultContentType(Format inp);
    void defaultAccept(Format inp);
    void setUrl(const Url& inp);
    void setHeaderOpts();
    void setQueryOpts();
    void addHeader(const ConOption& inp);
    void addQuery(const ConOption& inp) ;
    void addHeader(ConOption&& inp);
    void addQuery(ConOption&& inp);
    void setPostReq();
    void setDeleteReq();
    void setGetReq() ;
    void setPutReq();
    void setPatchReq();
    void setPostField(const std::string& inp);
    void setPostField(const VPack data) ;
    void setBuffer();
    void setAsynchronous(const bool bAsync = false);
    void run();
    bool isRunning() const;
    VPack result() const;
    long responseCode();

    static ServicePtr initService();
private:
    enum RequestType: uint16_t {    ReqDelete = 0
        , ReqGet =1
        , ReqPost = 2
        , ReqPut = 3
        , ReqPatch = 5
    };
  void setReqType();
  void gotConnection(const boost::system::error_code &err,
                     tcp::resolver::iterator endpoint_iterator);
  HeaderType::MsgId nextID();

    static HeaderType::MsgId _idKey;

    Service _service;
    tcp::socket _socket;

    HeaderType::MsgId _msgId;
    HeaderList _headers;
    QueryList _queries;
    Mode _mode;
    Builder _request;
    Chunks _chnks;
};

inline   Connection& VppConnection::operator=(const Protocol)
{
    return *this;
};

inline  void VppConnection::defaultContentType(Format ) {};

inline  void VppConnection::defaultAccept(Format ) {};

}
}

#endif // VPPCONNECTION_H
