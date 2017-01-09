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
/// @author Frank Celler
/// @author Jan Uhde
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGO_CXX_DRIVER_VST_CONNECTION_H
#define ARANGO_CXX_DRIVER_VST_CONNECTION_H 1

#include <functional>
#include <fuerte/connection_interface.h>
#include "asio/asio.h"
#include "asio/Socket.h"
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/streambuf.hpp>

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {

class isChunkComplete
{
public:
  explicit isChunkComplete(){}
  template <typename Iterator>
  //is iterator a random access iterator?
  std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
  {
    auto len = std::distance(begin, end);
    if(len > sizeof(uint32_t)){
      std::size_t clen;
      //what is the iterator exactly????
      std::memcpy(&clen, &*begin, sizeof(uint32_t));
      if(len >= clen){
        return std::make_pair(begin+clen, true);
      }
    }
    return std::make_pair(end, false);
  }
};

}}}}

namespace boost { namespace asio {
  template <> struct is_match_condition<::arangodb::fuerte::v1::vst::isChunkComplete>
    : public boost::true_type {};
}}

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {

template <typename T>
class VstConnection : public ConnectionInterface {
 public:
  using SocketType = T;
  VstConnection(detail::ConnectionConfiguration);

 public:
  void start() override {}

  void sendRequest(std::unique_ptr<Request>
                  ,OnErrorCallback
                  ,OnSuccessCallback) override;

  std::unique_ptr<Response> sendRequest(std::unique_ptr<Request>) override {
    return std::unique_ptr<Response>(nullptr);
  }

 //handler to be posted to loop
 //this handler call their handle counterpart on completeion
 void startWrite(){}
 void startRead(){
   // Set a deadline for the read operation.
   _deadline.expires_from_now(boost::posix_time::seconds(30));

   // Start an asynchronous operation to read a until a vst message/chunk is complete
   asio::socketcommon::doAsyncReadUntil(_socket, _input_buffer, isChunkComplete() /*add memeber*/,
                        std::bind(&VstConnection::handleRead, this, std::placeholders::_1));

 }

 void handleRead(){}
 void handleWrite(){}

 private:
  std::shared_ptr<asio::Loop> _asioLoop;
  ::boost::asio::io_service* _ioService;
  detail::ConnectionConfiguration _configuration;
  std::unique_ptr<asio::Socket> _socket;
  ::boost::asio::deadline_timer _deadline;
  ::boost::asio::streambuf _input_buffer;
  //boost::posix_time::milliseconds _keepAliveTimeout;
  //boost::asio::deadline_timer _keepAliveTimer;
  //bool const _useKeepAliveTimer;
  //bool _keepAliveTimerActive;
  //bool _closeRequested;
  //std::atomic_bool _abandoned;

};

}}}}
#endif
