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

namespace arangodb {

namespace dbnodejs {

Nan::Persistent<v8::Function> Connection::_constructor;

//
// Default to an http connection type
//
Connection::Connection() {
  typedef arangodb::dbinterface::Connection DbCon;
  typedef arangodb::dbinterface::HttpConnection HttpCon;
  DbCon* ptr = new HttpCon();
  _pConnection = Ptr{ptr};
}

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
  tpl->InstanceTemplate()->SetInternalFieldCount(2);

  // TODO Register prototypes for Javascript functions
  //
  Nan::SetPrototypeMethod(tpl, "Address", Connection::Address);
  Nan::SetPrototypeMethod(tpl, "Run", Connection::Run);

  // Provides Javascript with GetVersion constructor
  // function
  _constructor.Reset(tpl->GetFunction());
  target->Set(Nan::New("Connection").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(Connection::Address) {
  void* ptr = ObjectWrap::Unwrap<Connection>(info.Holder());
  std::cout << "Address : " << ptr << std::endl;
}

NAN_METHOD(Connection::Run) {
  typedef arangodb::dbinterface::HttpConnection HttpConnection;
  typedef arangodb::dbinterface::Connection::VPack VPack;
  Connection* pCon = ObjectWrap::Unwrap<Connection>(info.Holder());
  Connection::Ptr pLibCon = pCon->_pConnection;
  pLibCon->setAsynchronous(false);
  pLibCon->run();
  VPack vpack = pLibCon->result();
  std::string res = HttpConnection::json(vpack);
  info.GetReturnValue().Set(
        Nan::New(res.c_str()).ToLocalChecked());
  //info.GetReturnValue().Set(Nan::New("Need to get result").ToLocalChecked());
}

//
// Static function to create a Javascript GetVersion object
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
    std::cout << "Object factory method" << std::endl;
    // Invoked as plain function `GetVersion(...)`,
    // turn into construct call.
    std::cout << "Parameters : " << info.Length() << std::endl;
    v8::Local<v8::Value> argv[1] = {info[0]};
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(_constructor);
    info.GetReturnValue().Set(cons->NewInstance(1, argv));
  }
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
  v8::Local<v8::Function> cons = Nan::New<v8::Function>(_constructor);
  v8::Local<v8::Object> instance = cons->NewInstance(argc, argv);

  return scope.Escape(instance);
}
}
}
