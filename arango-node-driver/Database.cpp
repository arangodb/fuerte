#include <iostream>
#include <memory>

#include "Database.h"
#include "Server.h"

namespace arangodb { namespace dbnodejs {

NAN_METHOD(Database::New) {
  if (info.IsConstructCall()) {

    // warum plain pointer wer loescht?
    int nArgs = info.Length();
    if (nArgs !=2 ) {
      Nan::ThrowTypeError("Not 2 Arguments");
    }

    v8::Local<v8::Value> val = info[0];
    if (!val->IsString()) {
      Nan::ThrowTypeError("String parameter required");
    }

    if (info[0]->IsObject() && info[1]->IsString()){

      Nan::ObjectWrap::Unwrap<Server>(info[0]);

      //find out how to convert
      Nan::To<double>(info[1]);
      Database* obj = new Database(info[0], Nan::To<v8::String>(info[1]));

    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }
  } else {
    // Invoked as plain function `GetVersion(...)`,
    // turn into construct call.
    enum { MAX_ARGS = 10 };
    int argc = info.Length();
    if (argc > MAX_ARGS) {
      argc = MAX_ARGS;
    }
    v8::Local<v8::Value> argv[MAX_ARGS];
    for (int i = 0; i < argc; ++i) {
      argv[i] = info[i];
    }
    v8::Local<v8::Function> cons = Nan::New(constructor());
    auto val = cons->NewInstance(argc, argv);
    info.GetReturnValue().Set(Nan::NewInstance(cons,argc,argv).ToLocalChecked());
  }
}


}}
