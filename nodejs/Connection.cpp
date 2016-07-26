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
#include <fuerte/HttpConnection.h>
#include <iostream>

#include "Connection.h"
#include "Server.h"

namespace arangodb {

namespace dbnodejs {

Nan::Persistent<v8::Function> Connection::_constructor;

//
// Default to an http connection type
//
Connection::Connection() {}

Connection::Connection(const Ptr inp) { _pConnection = inp; }

//
// Required for all JavaScript Node.js implementations
//
// Initialises the Connection object functionality provided by
// the DbVer node
//
//  void Connection::Init(v8::Local<v8::Object> exports)
//
NAN_MODULE_INIT(Connection::Init) {
  Nan::HandleScope scope;

  // Create Javascript new Connection object template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Connection").ToLocalChecked());

  // Only 1 internal field required for this wrapped class
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // TODO Register prototypes for Javascript functions
  //
  Nan::SetPrototypeMethod(tpl, "EnumValues"
                          ,Connection::EnumValues);
  Nan::SetPrototypeMethod(tpl, "Address", Connection::Address);
  Nan::SetPrototypeMethod(tpl, "Result", Connection::Result);
  Nan::SetPrototypeMethod(tpl, "Run", Connection::Run);
  Nan::SetPrototypeMethod(tpl, "IsRunning"
                          ,Connection::IsRunning);
  Nan::SetPrototypeMethod(tpl, "SetAsynchronous"
                          ,Connection::SetAsynchronous);
  Nan::SetPrototypeMethod(tpl, "ResponseCode"
                          ,Connection::ResponseCode);

  // Provides Javascript with Connection constructor
  // function
  _constructor.Reset(tpl->GetFunction());
  target->Set(Nan::New("Connection").ToLocalChecked()
              , tpl->GetFunction());
}

NAN_METHOD(Connection::EnumValues) {
  typedef arangodb::dbinterface::Connection::Format Format;
  v8::Local<v8::Object> obj = Nan::New<v8::Object>();

  Nan::Set(obj,Nan::New("JSon").ToLocalChecked()
           , Nan::New<v8::Number>(
             static_cast<int>(Format::Json)));
  Nan::Set(obj,Nan::New("VPack").ToLocalChecked()
           , Nan::New<v8::Number>(
             static_cast<int>(Format::VPack)));
  info.GetReturnValue().Set(obj);
}

NAN_METHOD(Connection::Address) {
  void* ptr = ObjectWrap::Unwrap<Connection>(info.Holder());
  std::cout << "Address : " << ptr << std::endl;
}

NAN_METHOD(Connection::SetAsynchronous) {
  Connection* pCon = ObjectWrap::Unwrap<Connection>(info.Holder());
  int nArgs = info.Length();
  if (nArgs > 1) {
    Nan::ThrowTypeError("Too many arguments");
    return;
  }
  bool bAsync = false;
  if (nArgs == 1) {
    v8::Local<v8::Value> val = info[0];
    if (!val->IsBoolean()) {
      Nan::ThrowTypeError("Boolean parameter required");
      return;
    }
    bAsync = val->BooleanValue();
  }
  Connection::Ptr pLibCon = pCon->_pConnection;
  pLibCon->setAsynchronous(bAsync);
  info.GetReturnValue().Set(bAsync);
}

NAN_METHOD(Connection::IsRunning) {
  if (info.Length() > 0) {
    Nan::ThrowTypeError("No arguments required");
    return;
  }
  Connection* pCon = ObjectWrap::Unwrap<Connection>(info.Holder());
  Connection::Ptr pLibCon = pCon->_pConnection;
  info.GetReturnValue().Set(pLibCon->isRunning());
}

NAN_METHOD(Connection::Run) {
  typedef arangodb::dbinterface::HttpConnection HttpConnection;
  typedef arangodb::dbinterface::Connection::VPack VPack;
  if (info.Length() > 0) {
    Nan::ThrowTypeError("No arguments required");
    return;
  }
  Connection* pCon = ObjectWrap::Unwrap<Connection>(info.Holder());
  Connection::Ptr pLibCon = pCon->_pConnection;
  pLibCon->run();
}

NAN_METHOD(Connection::Result) {
  typedef arangodb::dbinterface::HttpConnection HttpConnection;
  typedef arangodb::dbinterface::Connection::VPack VPack;
  if (info.Length() > 0) {
    Nan::ThrowTypeError("No arguments required");
    return;
  }
  Connection* pCon = ObjectWrap::Unwrap<Connection>(info.Holder());
  Connection::Ptr pLibCon = pCon->_pConnection;
  VPack vpack = pLibCon->result();
  std::string res = HttpConnection::json(vpack);
  info.GetReturnValue().Set(
        Nan::New(res.c_str()).ToLocalChecked());
}

//
// Static function to create a new Javascript Connection object
//
// This is called from Javascript as follows
// var obj = new node.GetVersion()
//
//  void Connection::New(
//    const Nan::FunctionCallbackInfo<v8::Value>& info)
NAN_METHOD(Connection::New) {
  if (info.IsConstructCall()) {
    Connection* obj = new Connection();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `Connection(...)`,
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
    v8::Local<v8::Function> cons =
        Nan::New<v8::Function>(_constructor);
    auto val = cons->NewInstance(argc, argv);
    info.GetReturnValue().Set(val);
  }
}

NAN_METHOD(Connection::ResponseCode) {
  Connection* pCon = ObjectWrap::Unwrap<Connection>(info.Holder());
  Connection::Ptr pLibCon = pCon->_pConnection;
  if (info.Length() > 0) {
    Nan::ThrowTypeError("No arguments required");
    return;
  }
  std::int32_t res = static_cast<std::int32_t>(
        pLibCon->responseCode());
  info.GetReturnValue().Set(res);
}

void Connection::SetReturnValue(
    const Nan::FunctionCallbackInfo<v8::Value>& info, Ptr inp) {
  v8::Local<v8::Value> argv[0];
  v8::Local<v8::Function> cons =
      Nan::New<v8::Function>(_constructor);
  v8::Local<v8::Object> val = cons->NewInstance(0, argv);
  Connection* pCon = ObjectWrap::Unwrap<Connection>(val);
  pCon->_pConnection = inp;
  info.GetReturnValue().Set(val);
}

//
// Creates a new instance of Connection
//
// NOTE : This is not a Javascript Connection method
//
v8::Local<v8::Value> Connection::NewInstance(v8::Local<v8::Value> arg) {
  Nan::EscapableHandleScope scope;

  const unsigned argc = 1;
  v8::Local<v8::Value> argv[argc] = {arg};
  v8::Local<v8::Function> cons =
      Nan::New<v8::Function>(_constructor);
  v8::Local<v8::Object> instance = cons->NewInstance(argc, argv);

  return scope.Escape(instance);
}
}
}
