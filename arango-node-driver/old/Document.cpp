#include <iostream>
#include <memory>

#include "Document.h"
#include "Database.h"
#include "Connection.h"
#include "Collection.h"

namespace arangodb { namespace dbnodejs {

NAN_METHOD(Document::New) {
  if (info.IsConstructCall()) {

    if (info[0]->IsString()){
      Document* obj = new Document(*Nan::Utf8String(info[0]));

      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {
      Nan::ThrowTypeError("invalid parameters");
    }
  } else {
    int argc = info.Length();
    if (argc != 1) {
      argc = 2;
    }
    v8::Local<v8::Value> argv[1];
    for (int i = 0; i < argc; ++i) {
      argv[i] = info[i];
    }
    v8::Local<v8::Function> cons = Nan::New(constructor());
    info.GetReturnValue().Set(Nan::NewInstance(cons,argc,argv).ToLocalChecked());
  }
}

NAN_METHOD(Document::create) {
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Not 1 Argument");
  }
  Nan::ObjectWrap::Unwrap<Document>(info.Holder())->_cppDocument.create(//col,conn,opts
    Nan::ObjectWrap::Unwrap<Collection>(info[0]->ToObject())->cppClass(),
    Nan::ObjectWrap::Unwrap<Connection>(info[0]->ToObject())->cppClass()
    //VPack - std::shared_ptr<arangodb::velocypack::Buffer<uint8_t>> optional param
    //options ...
  );
}

}}
