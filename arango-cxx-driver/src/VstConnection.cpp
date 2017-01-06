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

#include "VstConnection.h"
#include "helper.h"
#include <fuerte/loop.h>

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {

using namespace arangodb::fuerte::detail;

HttpConnection::HttpConnection( ConnectionConfiguration configuration)
    : _asioLoop(LoopProvider::getProvider().getAsioLoop())
    , _ioService(_asioLoop->getIoService())
    , _configuration(configuration)
    {
  _configuration._ssl
  _configuration._port //remote
  _configuration._host //remote


}

void HttpConnection::sendRequest(std::unique_ptr<Request> request,
                                 OnErrorCallback onError,
                                 OnSuccessCallback onSuccess){
  Callbacks callbacks(onSuccess, onError);

  //_communicator->queueRequest(destination, std::move(request), callbacks);
  //

}

}}}}
