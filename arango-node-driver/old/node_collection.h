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
#ifndef FUERTE_NODE_COLLECTION_H
#define FUERTE_NODE_COLLECTION_H

#include "node_upstream.h"
#include <fuerte/collection.h>

namespace arangodb { namespace fuerte { namespace js {

class Collection : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("Collection").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(tpl, "create", Collection::create);
    constructor().Reset(tpl->GetFunction());
    target->Set( Nan::New("Collection").ToLocalChecked() , tpl->GetFunction()); //put in module init?!
  }

  static NAN_METHOD(New);
  static NAN_METHOD(create);

  std::shared_ptr<arangodb::fuerte::Collection>& cppClass() {
    return _cppClass;
  }
 private:
   std::shared_ptr<arangodb::fuerte::Collection> _cppClass;

  static Nan::Persistent<v8::Function>& constructor(){
    static Nan::Persistent<v8::Function> ctor;
    return ctor;
  }

};

}}}
#endif  // FUERTE_NODEDATABASE_H
