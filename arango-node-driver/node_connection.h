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
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef FUERTE_NODE_CONNECTION_H
#define FUERTE_NODE_CONNECTION_H

#include <iostream>
#include "node_upstream.h"
namespace arangodb { namespace fuerte { namespace js {

class NConnectionBuilder : public Nan::ObjectWrap {
  friend class NConnection;
  NConnectionBuilder(): _cppClass(){}

public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("ConnectionBuilder").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1); //should be equal to the number of data members
    Nan::SetPrototypeMethod(tpl, "requestsLeft", NConnectionBuilder::connect);
    Nan::SetPrototypeMethod(tpl, "connect", NConnectionBuilder::connect);
    Nan::SetPrototypeMethod(tpl, "host", NConnectionBuilder::host);
    Nan::SetPrototypeMethod(tpl, "async", NConnectionBuilder::async);
    Nan::SetPrototypeMethod(tpl, "user", NConnectionBuilder::user);
    Nan::SetPrototypeMethod(tpl, "password", NConnectionBuilder::password);
    constructor().Reset(tpl->GetFunction());
    target->Set( Nan::New("ConnectionBuilder").ToLocalChecked() , tpl->GetFunction()); //put in module init?!
  }

  static NAN_METHOD(New);
  static NAN_METHOD(reqestsLeft);
  static NAN_METHOD(connect);
  static NAN_METHOD(host);
  static NAN_METHOD(async);
  static NAN_METHOD(user);
  static NAN_METHOD(password);

  arangodb::fuerte::ConnectionBuilder& cppClass() {
    return _cppClass;
  }

private:
  arangodb::fuerte::ConnectionBuilder _cppClass;

  static Nan::Persistent<v8::Function>& constructor(){
    static Nan::Persistent<v8::Function> ctor;
    return ctor;
  }

};

class NConnection : public Nan::ObjectWrap {
public:
  friend class NConnectionBuilder;
  NConnection(): _cppClass(){}
  NConnection(std::shared_ptr<::fu::Connection> conn): _cppClass(std::move(conn)){}

  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("Connection").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(tpl, "sendRequest", NConnection::sendRequest);
    constructor().Reset(tpl->GetFunction());
    target->Set( Nan::New("Connection").ToLocalChecked() , tpl->GetFunction()); //put in module init?!
  }

  static NAN_METHOD(New);
  static NAN_METHOD(sendRequest);

  std::shared_ptr<::fu::Connection>& cppClass() {
    return _cppClass;
  }
private:
  std::shared_ptr<::fu::Connection> _cppClass;

  static Nan::Persistent<v8::Function>& constructor(){
    static Nan::Persistent<v8::Function> ctor;
    return ctor;
  }

};

}}}
#endif

