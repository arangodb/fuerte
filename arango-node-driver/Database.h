#ifndef FUERTE_NODE_DATABASE_H
#define FUERTE_NODE_DATABASE_H

#include <fuerte/Server.h>
#include <fuerte/Database.h>
#include <nan.h>

namespace arangodb { namespace dbnodejs {

class Database : public Nan::ObjectWrap {
 public:
  // see https://github.com/nodejs/nan/blob/master/doc/object_wrappers.md#api_nan_object_wrap
  // for explanations
  static NAN_MODULE_INIT(Server::Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("Database").ToLocalChecked());

    // Only 1 internal field required for this wrapped class ( _cppDatabase )
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    //Nan::SetPrototypeMethod(tpl, "makeConnection", Server::makeConnection);
    //Nan::SetPrototypeMethod(tpl, "version", Server::version);

    constructor().Reset(Nan::GetFunction(tpl).toLocalChecked());

    target->Set( Nan::New("Database").ToLocalChecked()
               , Nan::GetFunction(tpl).toLocalChecked()
               );
  }

  static NAN_METHOD(New);
  //static NAN_METHOD(create);

 private:
  Database::Database(std::shared_ptr<arangodb::dbinterface::Server> const& server, const std::string name)
    : _cppDatabase(std::make_shared<arangodb::dbinterface::Server>(server, name))
    {}

  static Nan::Persistent<v8::Function>& constructor(){
    static Nan::Persistent<v8::Function> ctor;
    return ctor;
  }

  //only field in this class
  std::shared_ptr<arangodb::dbinterface::Server> _cppDatabase;
};

}}
#endif  // FUERTE_NODESERVER_H

