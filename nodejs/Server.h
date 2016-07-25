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
#ifndef FUERTE_NODE_SERVER_H
#define FUERTE_NODE_SERVER_H

#include <fuerte/Server.h>
#include <nan.h>

namespace arangodb {

namespace dbnodejs {

class Server : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  static NAN_METHOD(version);
  static NAN_METHOD(makeConnection);

 private:
  Server();
  Server(const std::string url);
  static Server *Create(const
                        Nan::FunctionCallbackInfo<v8::Value>
                        &info);

  typedef arangodb::dbinterface::Server LibType;
  typedef LibType::SPtr LibPtr;
  LibPtr _pServer;
  static Nan::Persistent<v8::Function> _constructor;
};
}
}

#endif  // FUERTE_NODESERVER_H
