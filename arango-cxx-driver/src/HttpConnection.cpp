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
////////////////////////////////////////////////////////////////////////////////

#include "HttpConnection.h"

namespace arangodb {
namespace fuerte {
inline namespace v1 {
namespace http {
using namespace arangodb::fuerte::detail;

HttpConnection::HttpConnection(std::shared_ptr<HttpCommunicator> communicator,
                               ConnectionConfiguration configuration)
    : _communicator(std::move(communicator)), _configuration(configuration) {}

void HttpConnection::sendRequest(std::unique_ptr<Request> request,
                                 OnErrorCallback onError,
                                 OnSuccessCallback onSuccess){
  Callbacks callbacks(onSuccess, onError);

  Destination destination =
      (_configuration._ssl ? "https://" : "http://") + _configuration._host +
      ":" + std::to_string(_configuration._port) + request->messageHeader.requestPath;

  auto const& parameter = request->messageHeader.parameter;

  if (parameter && !parameter.get().empty()) {
    std::string sep = "?";

    for (auto p : parameter.get()) {
#warning TODO url encode
      destination += sep + p.first + "=" + p.second;
      sep = "&";
    }

#warning TODO set header
#warning TODO authentication
  }

  _communicator->queueRequest(destination, std::move(request), callbacks);
}
}
}
}
}