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
    v8::Local<v8::Function> builder = Nan::New(constructor());
    info.GetReturnValue().Set(Nan::NewInstance(builder).ToLocalChecked());
 }
}

NAN_METHOD(NConnectionBuilder::connect){
  v8::Local<v8::Function> connFunction = Nan::New(NConnection::constructor());
  const int argc = 1;
  v8::Local<v8::Value> argv[argc] = {info.This()};
  auto connInstance = Nan::NewInstance(connFunction, argc, argv).ToLocalChecked();
  info.GetReturnValue().Set(connInstance);
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
    if(info[0]->IsObject()){ // NConnectionBuilderObject -- exact type check?
      obj->_cppClass = unwrap<NConnectionBuilder>(info[0])->_cppClass.connect();
    }
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
    v8::Local<v8::Function> requestProto = Nan::New(NConnection::constructor());
    auto reqObj = Nan::NewInstance(requestProto).ToLocalChecked();
    unwrap<NRequest>(reqObj)->setCppClass(std::move(creq));

    // wrap response
    v8::Local<v8::Function> responseProto = Nan::New(NConnection::constructor());
    auto resObj = Nan::NewInstance(requestProto).ToLocalChecked();
    unwrap<NResponse>(resObj)->setCppClass(std::move(cres));

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

    std::cout << std::endl << "req/res " << creq.get() << "/" << cres.get() << std::endl;
    // wrap request
    //v8::Local<v8::Function> requestProto = Nan::New(NConnection::constructor()); // with Nan
    v8::Local<v8::Function> requestProto = v8::Local<v8::Function>::New(iso,NConnection::constructor());
    //auto reqObj = Nan::NewInstance(requestProto).ToLocalChecked(); // with Nan
    auto reqObj = requestProto->NewInstance(iso->GetCurrentContext()).FromMaybe(v8::Local<v8::Object>());
    unwrap<NRequest>(reqObj)->setCppClass(std::move(creq));

    // wrap response
    v8::Local<v8::Function> responseProto = v8::Local<v8::Function>::New(iso,NConnection::constructor());
    auto resObj = responseProto->NewInstance(iso->GetCurrentContext()).FromMaybe(v8::Local<v8::Object>());
    unwrap<NResponse>(resObj)->setCppClass(std::move(cres));

    // build args
    const unsigned argc = 2;
    v8::Local<v8::Value> argv[argc] = { reqObj, resObj };

    // call and dispose
    std::cout << std::endl << "running js callback success, with iso: " << iso << std::endl;
    v8::Local<v8::Function> jsOnSucc = v8::Local<v8::Function>::New(iso,persOnSucc);
    //jsOnSucc->Call(v8::Null(iso), argc, argv);
    jsOnSucc->Call(iso->GetCurrentContext(), iso->GetCurrentContext()->Global(), argc, argv);
    persOnSucc.Reset(); // dispose of persistent
    //persOnSucc.Reset(iso,v8::Local<v8::Function>()); // dispose of persistent

  };

  //finally invoke the c++ callback
  auto req = std::unique_ptr<fu::Request>(new fu::Request(*(unwrap<NRequest>(info[0])->_cppClass))); //copy request so the old object stays valid
  unwrapSelf<NConnection>(info)->_cppClass->sendRequest(std::move(req), err, succ);
}

}}}
