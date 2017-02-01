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
#include <iostream>
#include <memory>

namespace arangodb { namespace fuerte { namespace js {

//NAN_METHOD(Connection::New) {
//  if (info.IsConstructCall()) {
//    if (info.Length() !=2 ) {
//      Nan::ThrowTypeError("Not 2 Arguments");
//    }
//    if (info[0]->IsObject() && info[1]->IsString()){
//      Connection* obj = new Connection(Nan::ObjectWrap::Unwrap<Database>(info[0]->ToObject())->cppClass() , *Nan::Utf8String(info[1]));
//      obj->Wrap(info.This());
//      info.GetReturnValue().Set(info.This());
//    } else {
//      Nan::ThrowTypeError("invalid parameters");
//    }
//  } else {
//    int argc = info.Length();
//    if (argc > 2) {
//      argc = 2;
//    }
//    v8::Local<v8::Value> argv[2];
//    for (int i = 0; i < argc; ++i) {
//      argv[i] = info[i];
//    }
//    v8::Local<v8::Function> cons = Nan::New(constructor());
//    info.GetReturnValue().Set(Nan::NewInstance(cons,argc,argv).ToLocalChecked());
//  }
//}
//
//NAN_METHOD(Connection::create) {
//  if (info.Length() != 1 ) {
//    Nan::ThrowTypeError("Not 1 Argument");
//  }
//  Nan::ObjectWrap::Unwrap<Connection>(info.Holder())->_cppConnection->create(
//    Nan::ObjectWrap::Unwrap<Connection>(info[0]->ToObject())->cppClass()
//  );
//}

}}}
