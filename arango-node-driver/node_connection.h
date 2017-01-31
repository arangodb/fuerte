#ifndef FUERTE_NODE_CONNECTION_H
#define FUERTE_NODE_CONNECTION_H

#include "node_upstream.h"
#include <fuerte/connection.h>

namespace arangodb { namespace fuerte { namespace js {

class Connection : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("Connection").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(tpl, "create", Connection::create);
    constructor().Reset(tpl->GetFunction());
    target->Set( Nan::New("Connection").ToLocalChecked() , tpl->GetFunction()); //put in module init?!
  }

  static NAN_METHOD(New);
  static NAN_METHOD(create);

  std::shared_ptr<arangodb::fuerte::Connection>& cppClass() {
    return _cppConnection;
  }
 private:
   std::shared_ptr<arangodb::fuerte::Connection> _cppConnection;
   Connection(std::shared_ptr<arangodb::fuerte::Database> const& database, const std::string name)
     : _cppConnection(std::make_shared<arangodb::fuerte::Connection>(database, name))
     {}

  static Nan::Persistent<v8::Function>& constructor(){
    static Nan::Persistent<v8::Function> ctor;
    return ctor;
  }

};

}}}
#endif

