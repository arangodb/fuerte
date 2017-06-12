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
/// @author John Bufton
/// @author Ewout Prangsma
////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef ARANGO_CXX_DRIVER_HTTP_CONNECTION_H
#define ARANGO_CXX_DRIVER_HTTP_CONNECTION_H 1

#include <fuerte/connection_interface.h>
#include <fuerte/loop.h>
#include <stdexcept>

#include "HttpCommunicator.h"

namespace arangodb {
namespace fuerte {
inline namespace v1 {
namespace http {

// HttpConnection implements a client->server connection using the HTTP protocol.
class HttpConnection : public ConnectionInterface {
 public:
  explicit HttpConnection(EventLoopService&, detail::ConnectionConfiguration const&);
  ~HttpConnection();

 public:
  // Start an asynchronous request.
  MessageID sendRequest(std::unique_ptr<Request>, OnErrorCallback, OnSuccessCallback) override;

  // Return the number of unfinished requests.
  std::size_t requestsLeft() override {
    return _communicator->requestsLeft();
  }

 private:
  std::shared_ptr<HttpCommunicator> _communicator;
  detail::ConnectionConfiguration _configuration;
};

}
}
}
}

#endif
