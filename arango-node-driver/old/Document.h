#ifndef FUERTE_NODE_DOCUMENT_H
#define FUERTE_NODE_DOCUMENT_H

#include <fuerte/Server.h>
#include <fuerte/Document.h>
#include <nan.h>

namespace arangodb { namespace dbnodejs {

class Document : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("Document").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(tpl, "create", Document::create);
    ftpl().Reset(tpl);
    constructor().Reset(tpl->GetFunction());
    target->Set(Nan::New("Document").ToLocalChecked() , tpl->GetFunction()); //put in module init?!
  }

  static NAN_METHOD(New);
  static NAN_METHOD(create);

  bool hasInstance(v8::Local<v8::Value> const& val){
    if(!val->IsObject()){ return false; }
    Nan::HandleScope scope;
    auto temp = Nan::New(ftpl());
    return temp->HasInstance(val->ToObject());
  }

  arangodb::dbinterface::Document& cppClass() {
    return _cppDocument;
  }


 private:
   Document(const std::string& name)
     : _cppDocument(name)
     {}

  static Nan::Persistent<v8::Function>& constructor(){
    static Nan::Persistent<v8::Function> ctor;
    return ctor;
  }

  static Nan::Persistent<v8::FunctionTemplate>& ftpl(){
    static Nan::Persistent<v8::FunctionTemplate> ftpl;
    return ftpl;
  }

  arangodb::dbinterface::Document _cppDocument;
};

}}
#endif  // FUERTE_NODEDATABASE_H

