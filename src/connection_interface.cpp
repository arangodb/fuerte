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
/// @author Ewout Prangsma
////////////////////////////////////////////////////////////////////////////////

#include <mutex>

#include <fuerte/connection_interface.h>
#include <fuerte/FuerteLogger.h>

namespace arangodb {
namespace fuerte {
inline namespace v1 {

std::unique_ptr<Response> ConnectionInterface::sendRequest(std::unique_ptr<Request> request){
  FUERTE_LOG_TRACE << "start sync request" << std::endl;

  std::mutex mutex;
  std::condition_variable conditionVar;
  bool done = false;

  auto rv = std::unique_ptr<Response>(nullptr);
  auto onError  = [&](::arangodb::fuerte::v1::Error error, std::unique_ptr<Request> request, std::unique_ptr<Response> response){
    FUERTE_LOG_TRACE << "sendRequest (sync): onError" << std::endl;
    rv = std::move(response);
    {
      std::unique_lock<std::mutex> lock(mutex);
      done = true;
    }
    conditionVar.notify_one();
  };

  auto onSuccess  = [&](std::unique_ptr<Request> request, std::unique_ptr<Response> response){
    FUERTE_LOG_TRACE << "sendRequest (sync): onSuccess" << std::endl;
    rv = std::move(response);
    {
      std::unique_lock<std::mutex> lock(mutex);
      done = true;
    }
    conditionVar.notify_one();
  };

  {
    // Start asynchronous request
    std::unique_lock<std::mutex> lock(mutex);
    sendRequest(std::move(request),onError,onSuccess);

    // Wait for request to finish.
    FUERTE_LOG_TRACE << "sendRequest (sync): before wait" << std::endl;
    conditionVar.wait(lock, [&]{ return done; });
  }

  FUERTE_LOG_TRACE << "sendRequest (sync): done" << std::endl;
  return std::move(rv);
}

}
}
}
