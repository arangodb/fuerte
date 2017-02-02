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

#include "node_connection.h"
#include "node_request.h"
#include <iostream>
#include <memory>

namespace arangodb { namespace fuerte { namespace js {

///////////////////////////////////////////////////////////////////////////////
// NConnectionBuilder /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
NAN_METHOD(NConnectionBuilder::New) {
  if (info.IsConstructCall()) {
      NConnectionBuilder* obj = new NConnectionBuilder();
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
  } else {
    v8::Local<v8::Function> cons = Nan::New(constructor());
    info.GetReturnValue().Set(Nan::NewInstance(cons).ToLocalChecked());
 }
}

NAN_METHOD(NConnectionBuilder::connect){
  std::shared_ptr<fu::Connection> connection =  unwrapSelf<NConnectionBuilder>(info)->_cppClass.connect();
	NConnection* objConn = new NConnection(connection);
  auto connObj = Nan::New<v8::Object>();
  objConn->Wrap(connObj);
  info.GetReturnValue().Set(connObj);
}

NAN_METHOD(NConnectionBuilder::host){
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  unwrapSelf<NConnectionBuilder>(info)->_cppClass.host(to<std::string>(info[0]));
  info.GetReturnValue().Set(info.This());
}
NAN_METHOD(NConnectionBuilder::async){
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  unwrapSelf<NConnectionBuilder>(info)->_cppClass.async(to<bool>(info[0]));
  info.GetReturnValue().Set(info.This());
}
NAN_METHOD(NConnectionBuilder::user){
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  unwrapSelf<NConnectionBuilder>(info)->_cppClass.user(to<std::string>(info[0]));
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NConnectionBuilder::password){
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  unwrapSelf<NConnectionBuilder>(info)->_cppClass.password(to<std::string>(info[0]));
  info.GetReturnValue().Set(info.This());
}




///////////////////////////////////////////////////////////////////////////////
// NConnection ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
NAN_METHOD(NConnection::New) {
  if (info.IsConstructCall()) {
    NConnection* obj = new NConnection();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    v8::Local<v8::Function> cons = Nan::New(constructor());
    info.GetReturnValue().Set(Nan::NewInstance(cons).ToLocalChecked());
 }
}

NAN_METHOD(NConnection::sendRequest) {
  if (info.Length() != 3 ) {
    Nan::ThrowTypeError("Not 3 Arguments");
  }

  // get isolate - has node only one context??!?!? how do they work
  // context is probably a lighter version of isolate but does not allow threads
  v8::Isolate* iso = v8::Isolate::GetCurrent();

  auto jsOnErr = v8::Local<v8::Function>::Cast(info[1]);
  v8::Persistent<v8::Function> persOnErr;
  persOnErr.Reset(iso, jsOnErr);

  auto jsOnSucc = v8::Local<v8::Function>::Cast(info[2]);
  v8::Persistent<v8::Function> persOnSucc;
  persOnSucc.Reset(iso, jsOnSucc);

  fu::OnErrorCallback err = [iso,&persOnErr](unsigned err
                              ,std::unique_ptr<fu::Request> creq
                              ,std::unique_ptr<fu::Response> cres)
  {
    v8::HandleScope scope(iso);
    v8::Local<v8::Function> jsOnErr = v8::Local<v8::Function>::New(iso,persOnErr);

    // wrap request
    NRequest* objReq = new NRequest(std::move(creq));
    auto reqObj = Nan::New<v8::Object>();
    objReq->Wrap(reqObj);

    // wrap response
    NResponse* objRes = new NResponse(std::move(cres));
    auto resObj = Nan::New<v8::Object>();
    objRes->Wrap(reqObj);

    // build args
    const unsigned argc = 3;
    v8::Local<v8::Value> argv[argc] = { Nan::New<v8::Integer>(err), reqObj, resObj };

    // call and dispose
    jsOnErr->Call(v8::Null(iso), argc, argv);
    persOnErr.Reset(); // dispose of persistent
  };

  fu::OnSuccessCallback succ = [iso,&persOnSucc](std::unique_ptr<fu::Request> creq
                                 ,std::unique_ptr<fu::Response> cres)
  {
    v8::HandleScope scope(iso);
    v8::Local<v8::Function> jsOnSucc = v8::Local<v8::Function>::New(iso,persOnSucc);

    // wrap request
    NRequest* objReq = new NRequest(std::move(creq));
    auto reqObj = Nan::New<v8::Object>();
    objReq->Wrap(reqObj);

    // wrap response
    NResponse* objRes = new NResponse(std::move(cres));
    auto resObj = Nan::New<v8::Object>();
    objRes->Wrap(reqObj);

    // build args
    const unsigned argc = 2;
    v8::Local<v8::Value> argv[argc] = { reqObj, resObj };

    // call and dispose
    jsOnSucc->Call(v8::Null(iso), argc, argv);
    persOnSucc.Reset(); // dispose of persistent
  };

  //finally invoke the c++ callback
  auto req = std::unique_ptr<fu::Request>(new fu::Request(*(unwrap<NRequest>(info[0])->_cppClass))); //copy request so the old object stays valid
  unwrapSelf<NConnection>(info)->_cppClass->sendRequest(std::move(req), err, succ);
}

}}}
