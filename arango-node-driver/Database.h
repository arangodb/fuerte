#ifndef FUERTE_NODE_DATABASE_H
#define FUERTE_NODE_DATABASE_H

#include <fuerte/Server.h>
#include <fuerte/Database.h>
#include <nan.h>

namespace arangodb {

namespace dbnodejs {

class Database : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  static NAN_METHOD(create);

 private:
  Database::Database(std::shared_ptr<arangodb::dbinterface::Server> const& server, const std::string name);
  //static Server* Create(const Nan::FunctionCallbackInfo<v8::Value>& info);

  std::shared_ptr<arangodb::dbinterface::Server> _cppDatabase;
  static Nan::Persistent<v8::Function> _constructor;
};
}
}

#endif  // FUERTE_NODESERVER_H

