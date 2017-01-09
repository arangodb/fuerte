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

#include <fuerte/connection_interface.h>
#include "asio/asio.h"
#include "asio/Socket.h"

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {

class VstConnection : public ConnectionInterface {
 public:
  VstConnection(detail::ConnectionConfiguration);

 public:
  void start() override {}

  void sendRequest(std::unique_ptr<Request>
                  ,OnErrorCallback
                  ,OnSuccessCallback) override;

  std::unique_ptr<Response> sendRequest(std::unique_ptr<Request>) override {
    return std::unique_ptr<Response>(nullptr);
  }

 private:
  std::shared_ptr<asio::Loop> _asioLoop;
  ::boost::asio::io_service* _ioService;
  detail::ConnectionConfiguration _configuration;
  std::unique_ptr<asio::Socket> _peer;
  //boost::posix_time::milliseconds _keepAliveTimeout;
  //boost::asio::deadline_timer _keepAliveTimer;
  //bool const _useKeepAliveTimer;
  //bool _keepAliveTimerActive;
  //bool _closeRequested;
  //std::atomic_bool _abandoned;

};

}}}}
#endif
