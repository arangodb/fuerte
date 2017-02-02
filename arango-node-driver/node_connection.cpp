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

// NConnectionBuilder
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
	//wrap
  //info.GetReturnValue().Set(connection);
	throw std::logic_error("implement!");
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

NAN_METHOD(NConnection::sendRequest) {
  if (info.Length() != 3 ) {
    Nan::ThrowTypeError("Not 3 Arguments");
  }

  Request req = unwrap<NRequest>(info[0])->_cppClass;

  auto jsOnErr = v8::Local<v8::Function>::Cast(info[1]);
  auto jsOnSucc = v8::Local<v8::Function>::Cast(info[2]);

  ::v8::Isolate* iso = ::v8::Isolate::GetCurrent();
  v8::Local<::v8::Context> cont = iso->GetCurrentContext();
  v8::Local<::v8::Context> cont2 = iso->GetCurrentContext()->Global();
  v8::Context* contp = *cont2;

  fu::OnErrorCallback err = [iso](unsigned err
                              ,std::unique_ptr<fu::Request> creq
                              ,std::unique_ptr<fu::Response> cres)
  {

  };

  fu::OnSuccessCallback succ = [](std::unique_ptr<fu::Request> creq
                                 ,std::unique_ptr<fu::Response> cres)
  {};

  unwrapSelf<NConnection>(info)->_cppClass->sendRequest(req, err, succ);
}

}}}
