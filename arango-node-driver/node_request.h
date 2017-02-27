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

#ifndef FUERTE_NODE_REQUEST_H
#define FUERTE_NODE_REQUEST_H

#include "node_upstream.h"
namespace arangodb { namespace fuerte { namespace js {

class NConnection;

class NRequest : public Nan::ObjectWrap {
    friend class NConnection;
    NRequest(): _cppClass(new fu::Request){}
    NRequest(std::unique_ptr<fu::Request> re): _cppClass(std::move(re)){}

public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("Request").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1); //should be equal to the number of data members

    Nan::SetPrototypeMethod(tpl, "notNull", NRequest::notNull);

    Nan::SetPrototypeMethod(tpl, "addVPack", NRequest::addVPack);
    Nan::SetPrototypeMethod(tpl, "addBinary", NRequest::addBinary);

    Nan::SetPrototypeMethod(tpl, "setPath", NRequest::setPath);
    Nan::SetPrototypeMethod(tpl, "setDatabase", NRequest::setDatabase);
    Nan::SetPrototypeMethod(tpl, "setRestVerb", NRequest::setRestVerb);
    Nan::SetPrototypeMethod(tpl, "password", NRequest::setPassword);
    Nan::SetPrototypeMethod(tpl, "user", NRequest::setUser);
    Nan::SetPrototypeMethod(tpl, "setContentType", NRequest::setContentType);
    Nan::SetPrototypeMethod(tpl, "setAcceptType", NRequest::setAcceptType);

    Nan::SetPrototypeMethod(tpl, "addParameter", NRequest::addParameter);
    Nan::SetPrototypeMethod(tpl, "addMeta", NRequest::addMeta);


    constructor().Reset(tpl->GetFunction());
    target->Set( Nan::New("Request").ToLocalChecked() , tpl->GetFunction()); //put in module init?!
  }

  static NAN_METHOD(New);

  static NAN_METHOD(notNull);

  static NAN_METHOD(addVPack);
  static NAN_METHOD(addBinary);

  static NAN_METHOD(setPath);
  static NAN_METHOD(setDatabase);
  static NAN_METHOD(setRestVerb);
  static NAN_METHOD(setPassword);
  static NAN_METHOD(setUser);
  static NAN_METHOD(setContentType);
  static NAN_METHOD(setAcceptType);

  static NAN_METHOD(addParameter);
  static NAN_METHOD(addMeta);

  arangodb::fuerte::Request* cppClass() {
    return _cppClass.get();
  }

  void setCppClass(std::unique_ptr<arangodb::fuerte::Request> request) {
    _cppClass = std::move(request);
  }

private:
  std::unique_ptr<arangodb::fuerte::Request> _cppClass;

  static Nan::Persistent<v8::Function>& constructor(){
    static Nan::Persistent<v8::Function> ctor;
    return ctor;
  }

};


class NResponse : public Nan::ObjectWrap {
    friend class NConnection;
    NResponse(): _cppClass(new fu::Response){}
    NResponse(std::unique_ptr<fu::Response> re): _cppClass(std::move(re)){}

public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("Response").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1); //should be equal to the number of data members

    Nan::SetPrototypeMethod(tpl, "notNull", NResponse::notNull);

    Nan::SetPrototypeMethod(tpl, "getContentType", NResponse::getContentType);
    Nan::SetPrototypeMethod(tpl, "getAcceptType", NResponse::getAcceptType);
    Nan::SetPrototypeMethod(tpl, "getResponseCode", NResponse::getResponseCode);
    Nan::SetPrototypeMethod(tpl, "getMeta", NResponse::getMeta);
    Nan::SetPrototypeMethod(tpl, "payload", NResponse::payload);
    Nan::SetPrototypeMethod(tpl, "buffers", NResponse::buffers);

    constructor().Reset(tpl->GetFunction());
    target->Set( Nan::New("Response").ToLocalChecked() , tpl->GetFunction()); //put in module init?!
  }

  static NAN_METHOD(New);

  static NAN_METHOD(notNull);

  static NAN_METHOD(getContentType);
  static NAN_METHOD(getAcceptType);
  static NAN_METHOD(getResponseCode);
  static NAN_METHOD(getMeta);
  static NAN_METHOD(payload);
  static NAN_METHOD(buffers);

  arangodb::fuerte::Response* cppClass() {
    return _cppClass.get();
  }

  void setCppClass(std::unique_ptr<arangodb::fuerte::Response> response) {
    _cppClass = std::move(response);
  }

private:
  std::unique_ptr<arangodb::fuerte::Response> _cppClass;

  static Nan::Persistent<v8::Function>& constructor(){
    static Nan::Persistent<v8::Function> ctor;
    return ctor;
  }

};
}}}
#endif
