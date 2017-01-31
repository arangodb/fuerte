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
    return _cppCollection;
  }
 private:
   std::shared_ptr<arangodb::fuerte::Collection> _cppCollection;
   Collection(std::shared_ptr<arangodb::fuerte::Database> const& database, const std::string name)
     : _cppCollection(std::make_shared<arangodb::fuerte::Collection>(database, name))
     {}

  static Nan::Persistent<v8::Function>& constructor(){
    static Nan::Persistent<v8::Function> ctor;
    return ctor;
  }

};

}}}
#endif  // FUERTE_NODEDATABASE_H
