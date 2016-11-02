#include <iostream>
#include <memory>

#include "Database.h"
#include "Server.h"

namespace arangodb {

namespace dbnodejs {

Nan::Persistent<v8::Function> Server::_constructor;

Database::Database(std::shared_ptr<arangodb::dbinterface::Server> const& server, const std::string name)
  : _cppDatabase(std::make_shared<arangodb::dbinterface::Server>(server, name))
  {}


  NAN_MODULE_INIT(Server::Init) {
  Nan::HandleScope scope;
  // Prepare constructor template
  // Create Javascript new Server object template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Database").ToLocalChecked());

  // Only 1 internal field required for this wrapped class
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // TODO Register prototypes for Javascript functions
  //
  //Nan::SetPrototypeMethod(tpl, "makeConnection", Server::makeConnection);
  //Nan::SetPrototypeMethod(tpl, "version", Server::version);

  // Provides Javascript with GetVersion constructor
  // and destructor?
  _constructor.Reset(tpl->GetFunction());
  target->Set(Nan::New("Database").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(Server::New) {
  if (info.IsConstructCall()) {
    Server* obj = nullptr;

    // warum plain pointer wer loescht?
    int nArgs = info.Length();
    if (nArgs > 1) {
      Nan::ThrowTypeError("Too many arguments");
    }
    if (!nArgs) {
      // Default Server url http://127.0.0.1:8529
      obj = new Server();
    }
    v8::Local<v8::Value> val = info[0];
    if (!val->IsString()) {
      Nan::ThrowTypeError("String parameter required");
    }
    v8::String::Utf8Value tmp{val};
    std::string inp{*tmp};
    obj = new Server(inp);

    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
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
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(_constructor);
    auto val = cons->NewInstance(argc, argv);
    info.GetReturnValue().Set(val);
  }
}


}
}

