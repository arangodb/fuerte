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

#include "Connection.h"
#include "Server.h"

namespace arangodb {

namespace dbnodejs {

Nan::Persistent<v8::Function> Server::_constructor;

Server::Server() {_pServer = std::make_shared<PType>(PType{}); }

//
// Required for all JavaScript Node.js implementations
//
// Initialises the Connection object functionality provided by
// the DbVer node
//
// void Server::Init(v8::Local<v8::Object> target)
//
NAN_MODULE_INIT(Server::Init) {
  Nan::HandleScope scope;
  // Prepare constructor template
  // Create Javascript new Server object template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Server").ToLocalChecked());

  // No idea what number to pass here
  // Cannot find any useful explanation
  tpl->InstanceTemplate()->SetInternalFieldCount(2);

  // TODO Register prototypes for Javascript functions
  //
  Nan::SetPrototypeMethod(tpl, "makeConnection", Server::makeConnection);
  Nan::SetPrototypeMethod(tpl, "version", Server::version);

  // Provides Javascript with GetVersion constructor
  // and destructor?
  _constructor.Reset(tpl->GetFunction());
  target->Set(Nan::New("Server").ToLocalChecked(), tpl->GetFunction());
}

//
// Javascript Server.makeConnection
//
// Javascript returns a Connection object
//
// void Server::version(
//  const Nan::FunctionCallbackInfo<v8::Value> &info)
NAN_METHOD(Server::version) {
  Server* pSrv = ObjectWrap::Unwrap<Server>(info.Holder());
  if (info.Length() < 1) {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }
  v8::Local<v8::Object> obj = info[0]->ToObject();
  Connection* pCon = ObjectWrap::Unwrap<Connection>(obj);
  Connection::Ptr pLibCon = pCon->libConnection();
  Server::Ptr pLibSrv = pSrv->_pServer;
  pLibSrv->version(pLibCon);
}

//
// Javascript Server.makeConnection
//
// Javascript returns a Connection object
//
// void Server::makeConnection(
//  const Nan::FunctionCallbackInfo<v8::Value> &info)
NAN_METHOD(Server::makeConnection) {
  info.GetReturnValue().Set(Connection::NewInstance(info[0]));
}

//
// Static function to create a Javascript GetVersion object
//
// This is called from Javascript as follows
// var obj = new node.GetVersion()
//
NAN_METHOD(Server::New) {
  if (info.IsConstructCall()) {
    Server* obj = new Server();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `GetVersion(...)`,
    // turn into construct call.
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = {info[0]};
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(_constructor);
    info.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}
}
}
