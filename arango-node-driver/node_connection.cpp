#include "node_connection.h"
#include <iostream>
#include <memory>

namespace arangodb { namespace fuerte { namespace js {

NAN_METHOD(Collection::New) {
  if (info.IsConstructCall()) {
    if (info.Length() !=2 ) {
      Nan::ThrowTypeError("Not 2 Arguments");
    }
    if (info[0]->IsObject() && info[1]->IsString()){
      Collection* obj = new Collection(Nan::ObjectWrap::Unwrap<Database>(info[0]->ToObject())->cppClass() , *Nan::Utf8String(info[1]));
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {
      Nan::ThrowTypeError("invalid parameters");
    }
  } else {
    int argc = info.Length();
    if (argc > 2) {
      argc = 2;
    }
    v8::Local<v8::Value> argv[2];
    for (int i = 0; i < argc; ++i) {
      argv[i] = info[i];
    }
    v8::Local<v8::Function> cons = Nan::New(constructor());
    info.GetReturnValue().Set(Nan::NewInstance(cons,argc,argv).ToLocalChecked());
  }
}

NAN_METHOD(Collection::create) {
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Not 1 Argument");
  }
  Nan::ObjectWrap::Unwrap<Collection>(info.Holder())->_cppCollection->create(
    Nan::ObjectWrap::Unwrap<Connection>(info[0]->ToObject())->cppClass()
  );
}

}}
