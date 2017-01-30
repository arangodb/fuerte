#ifndef FUERTE_NODE_CONNECTION_H
#define FUERTE_NODE_CONNECTION_H

#include <fuerte/connection.h>
#include <nan.h>

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

  std::shared_ptr<arangodb::dbinterface::Collection>& cppClass() {
    return _cppCollection;
  }
 private:
   std::shared_ptr<arangodb::dbinterface::Collection> _cppCollection;
   Collection(std::shared_ptr<arangodb::dbinterface::Database> const& database, const std::string name)
     : _cppCollection(std::make_shared<arangodb::dbinterface::Collection>(database, name))
     {}

  static Nan::Persistent<v8::Function>& constructor(){
    static Nan::Persistent<v8::Function> ctor;
    return ctor;
  }

};

}}}
#endif

