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
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);

    tpl->SetClassName(Nan::New("Database").ToLocalChecked());

    // Only 1 internal field required for this wrapped class ( _cppDatabase ) / is this really meant by field?
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // add member functions accessible form node
    Nan::SetPrototypeMethod(tpl, "create", Database::create);

#ifdef GOOD //but not working
    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    target->Set( Nan::New("Database").ToLocalChecked()
               , Nan::GetFunction(tpl).ToLocalChecked()
               );
#else
    constructor().Reset(tpl->GetFunction());
    target->Set( Nan::New("Database").ToLocalChecked()
               , tpl->GetFunction()
               ); //put in module init?!
#endif

  }

  static NAN_METHOD(New);
  static NAN_METHOD(create);

  std::shared_ptr<arangodb::dbinterface::Database>& cppClass() {
    return _cppDatabase;
  }

 private:
  std::shared_ptr<arangodb::dbinterface::Database> _cppDatabase;

   Database(std::shared_ptr<arangodb::dbinterface::Server> const& server, const std::string name)
     : _cppDatabase(std::make_shared<arangodb::dbinterface::Database>(server, name))
     {}

  static Nan::Persistent<v8::Function>& constructor(){
    static Nan::Persistent<v8::Function> ctor;
    return ctor;
  }

};

}}
#endif  // FUERTE_NODEDATABASE_H

