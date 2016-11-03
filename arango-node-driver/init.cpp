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
#include <iostream>

#include "init.h"
#include "Connection.h"
#include "Server.h"
#include "Database.h"

namespace arangodb { namespace dbnodejs {

void InitAll(v8::Local<v8::Object> exports) {
  std::cout << "About to init classes" << std::endl;
  Connection::Init(exports);
  Server::Init(exports);
  Database::Init(exports);
}

}}

//
// Names the node and the function call to initialise
// the functionality it will provide
//
NODE_MODULE(arango_node_driver, arangodb::dbnodejs::InitAll);
