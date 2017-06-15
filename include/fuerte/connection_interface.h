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
/// @author Jan Christoph Uhde
/// @author Ewout Prangsma
////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef ARANGO_CXX_DRIVER_CONNECTION_INTERFACE
#define ARANGO_CXX_DRIVER_CONNECTION_INTERFACE

#include "message.h"
#include "FuerteLogger.h"
namespace arangodb { namespace fuerte { inline namespace v1 {

class ConnectionInterface {
  // this class is a member of the connection class and it provides
  // an interface so that the curl and asio implementaion can be hideen
  // via pointer to implementaton
public:
  ConnectionInterface() {}
  virtual ~ConnectionInterface() {}

  // Run a synchronous request
  virtual std::unique_ptr<Response> sendRequest(std::unique_ptr<Request>);
  // Start an asynchronous request
  virtual MessageID sendRequest(std::unique_ptr<Request>, RequestCallback) = 0;
  // Return the number of unfinished requests.
  virtual std::size_t requestsLeft() = 0;

  // Activate the connection.  
  virtual void start() {}
};

}}}
#endif
