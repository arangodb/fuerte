#include <iostream>
#include <memory>

#include "Database.h"
#include "Server.h"

namespace arangodb { namespace dbnodejs {

NAN_METHOD(Database::New) {
  if (info.IsConstructCall()) {
    if (info.Length() !=2 ) {
      Nan::ThrowTypeError("Not 2 Arguments");
    }
    if (info[0]->IsObject() && info[1]->IsString()){
      Database* obj = new Database(Nan::ObjectWrap::Unwrap<Server>(info[0])->_pServer , *Nan::Utf8String(info[1]));
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
    auto val = cons->NewInstance(argc, argv);
    info.GetReturnValue().Set(Nan::NewInstance(cons,argc,argv).ToLocalChecked());
  }
}


}}
