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

#include "HttpConnection.h"
#include <fuerte/helper.h> // url de/en-code
#include <fuerte/loop.h>

namespace arangodb {
namespace fuerte {
inline namespace v1 {
namespace http {

using namespace arangodb::fuerte::detail;

HttpConnection::HttpConnection(EventLoopService& eventLoopService, ConnectionConfiguration const& configuration)
    : Connection(eventLoopService, configuration),
    _communicator(eventLoopService.httpCommunicator()) {
  _communicator->addUser();
}

HttpConnection::~HttpConnection() { _communicator->delUser(); }

MessageID HttpConnection::sendRequest(std::unique_ptr<Request> request, RequestCallback callback) {
  std::string dbString = (request->header.database) ? std::string("/_db/") + request->header.database.get() : std::string("");
  Destination destination = (_configuration._ssl ? "https://" : "http://")
                          + _configuration._host
                          + ":"
                          + _configuration._port
                          + dbString
                          + request->header.path.get();

  auto const& parameter = request->header.parameter;

  if (parameter && !parameter.get().empty()) {
    std::string sep = "?";

    for (auto p : parameter.get()) {
      destination += sep + urlEncode(p.first) + "=" + urlEncode(p.second);
      sep = "&";
    }
  }
  #warning TODO authentication
  return _communicator->queueRequest(destination, std::move(request), callback);
  //create usefulid
}

}
}
}
}
