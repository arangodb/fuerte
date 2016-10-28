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

Server::Server() { _pServer = std::make_shared<LibType>(LibType{}); }

Server::Server(const std::string url) {
  _pServer = std::make_shared<LibType>(LibType{url});
}

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

  // Only 1 internal field required for this wrapped class
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

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
// Javascript returns a Connection object   -- FIXME it returns nothing
//
// void Server::version(
//  const Nan::FunctionCallbackInfo<v8::Value> &info)
NAN_METHOD(Server::version) {
  if (info.Length() != 1) {
    Nan::ThrowTypeError("Single Connnection object parameter required");
    return;
  }
  Server* pSrv = ObjectWrap::Unwrap<Server>(info.Holder());
  v8::Local<v8::Object> obj = info[0]->ToObject();
  Connection* pCon = ObjectWrap::Unwrap<Connection>(obj);
  Connection::Ptr pLibCon = pCon->libConnection();
  LibPtr pLibSrv = pSrv->_pServer;
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
  int nArgs = info.Length();
  if (nArgs > 0) {
    Nan::ThrowTypeError("Too many arguments");
    return;
  }
  Server* pSrv = ObjectWrap::Unwrap<Server>(info.Holder());
  LibPtr pLibSrv = pSrv->_pServer;
  auto libConPtr = pLibSrv->makeConnection();
  Connection::SetReturnValue(info, libConPtr);
}

Server* Server::Create(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  int nArgs = info.Length();
  if (nArgs > 1) {
    Nan::ThrowTypeError("Too many arguments");
    return nullptr;
  }
  if (!nArgs) {
    // Default Server url http://127.0.0.1:8529
    return new Server();
  }
  v8::Local<v8::Value> val = info[0];
  if (!val->IsString()) {
    Nan::ThrowTypeError("String parameter required");
    return nullptr;
  }
  v8::String::Utf8Value tmp{val};
  std::string inp{*tmp};
  return new Server(inp);
}

//
// Static function to create a Javascript Server object
//
// This is called from Javascript as follows
// var obj = new node.Server()
//
NAN_METHOD(Server::New) {
  if (info.IsConstructCall()) {
    Server* obj = Create(info);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `GetVersion(...)`,
    // turn into construct call.
    enum { MAX_ARGS = 10 };
    int argc = info.Length();
    if (argc > MAX_ARGS) {
      argc = MAX_ARGS;
    }
    v8::Local<v8::Value> argv[MAX_ARGS];
    for (int i = 0; i < argc; ++i) {
      argv[i] = info[i];
    }
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(_constructor);
    auto val = cons->NewInstance(argc, argv);
    info.GetReturnValue().Set(val);
  }
}
}
}
