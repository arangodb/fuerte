////////////////////////////////////////////////////////////////////////////////
/// @brief Nodejs example to get ArangoDB version using cURL.
///
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author John Bufton
/// @author Copyright 2016, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////
#include <cstring>
#include <iostream>
#include <string>

#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

#include "GetVersion.h"

//-------------------------------

//
// Initialise all the functionality provided by the
// DbVer node
//
void InitAll(v8::Local<v8::Object> exports) { GetVersion::Init(exports); }

//
// Names the node and the function call to initialise
// the functionality it will provide
//
NODE_MODULE(AsyncDbVer, InitAll)

//-------------------------------

namespace {

//
// CurlInit class
//
// A global instance of this ensures cURL is initialised
// and terminated as required when used
//
class CurlInit {
 public:
  CurlInit();
  ~CurlInit();
};

CurlInit::CurlInit() { curlpp::initialize(); }

CurlInit::~CurlInit() { curlpp::terminate(); }

CurlInit init;
}

//-------------------------------

//
// static variable required for GetVersion construction and
// destruction?
//
Nan::Persistent<v8::Function> GetVersion::_constructor;

void GetVersion::errorMessage(const std::string& inp) {
  _multi.remove(&_easy);
  _buf.WriteMemoryCallback(inp.c_str(), inp.length(), 1);
}

bool GetVersion::config() {
  static const std::string url{"http://127.0.0.1:8529/_api/version"};
  try {
    _easy.reset();
    _buf.reset();
    curlpp::types::WriteFunctionFunctor functor{&_buf,
                                                &Buffer::WriteMemoryCallback};
    _easy.setOpt(curlpp::options::Url(url));
    _easy.setOpt(curlpp::options::WriteFunction(functor));
    _multi.add(&_easy);
    return true;
  } catch (curlpp::LogicError& e) {
    errorMessage(std::string(e.what()));
    return false;
  } catch (curlpp::RuntimeError& e) {
    errorMessage(std::string(e.what()));
    return false;
  }
  return false;
}

void GetVersion::checkMsgs() {
  curlpp::Multi::Msgs msgs = _multi.info();

  if (!msgs.empty()) {
    auto msg = msgs.front();

    if (msg.second.msg == CURLMSG_DONE) {
      CURLcode code = msg.second.code;

      if (code != CURLE_OK) {
        std::string msg{curl_easy_strerror(code)};
        errorMessage(msg);
        return;
      }

      _multi.remove(&_easy);
    }
  }
}

bool GetVersion::process() {
  try {
    int nbLeft;

    while (!_multi.perform(&nbLeft)) {
      ;
    }

    checkMsgs();

    return nbLeft == 0;
  } catch (curlpp::LogicError& e) {
    errorMessage(std::string(e.what()));
    return true;
  } catch (curlpp::RuntimeError& e) {
    errorMessage(std::string(e.what()));
    return true;
  }
}

//
// Gets the ArangoDB version using cURL
//
std::string GetVersion::version() {
  if (config()) {
    while (!process())
      ;
  }
  return _buf.result();
}

//
// Required for all JavaScript Node.js implementations
//
// Initialises the GetVersion object functionality provided by
// the AsyncDbVer node
//
void GetVersion::Init(v8::Local<v8::Object> exports) {
  Nan::HandleScope scope;
  // Prepare constructor template
  // Create New Javascript function for GetVersion
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("GetVersion").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Register prototype for Javascript GetVerson
  // member functions
  Nan::SetPrototypeMethod(tpl, "version", GetVersion::Version);
  Nan::SetPrototypeMethod(tpl, "process", GetVersion::Process);
  Nan::SetPrototypeMethod(tpl, "config", GetVersion::Config);
  Nan::SetPrototypeMethod(tpl, "result", GetVersion::Result);
  // Provides Javascript with GetVersion constructor
  // and destructor?
  _constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("GetVersion").ToLocalChecked(), tpl->GetFunction());
}

//
// Static function that is called from Javascript
// to implement GetVersion obj.result()
//
void GetVersion::Result(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  GetVersion* obj = ObjectWrap::Unwrap<GetVersion>(info.Holder());
  std::string res = obj->result();
  info.GetReturnValue().Set(Nan::New(res).ToLocalChecked());
}

//
// Static function that is called from Javascript
// to implement GetVersion obj.process()
//
void GetVersion::Process(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  GetVersion* obj = ObjectWrap::Unwrap<GetVersion>(info.Holder());
  bool res = obj->process();
  info.GetReturnValue().Set(res);
}

//
// Static function that is called from Javascript
// to implement GetVersion obj.process()
//
void GetVersion::Config(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  GetVersion* obj = ObjectWrap::Unwrap<GetVersion>(info.Holder());
  bool res = obj->config();
  info.GetReturnValue().Set(res);
}

//
// Static function that is called from Javascript
// to implement GetVersion obj.version()
//
void GetVersion::Version(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  GetVersion* obj = ObjectWrap::Unwrap<GetVersion>(info.Holder());
  std::string res = obj->version();
  info.GetReturnValue().Set(Nan::New(res).ToLocalChecked());
}

//
// Static function to create a Javascript GetVersion object
//
// This is called from Javascript as follows
// var obj = new node.GetVersion()
//
void GetVersion::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (info.IsConstructCall()) {
    GetVersion* obj = new GetVersion();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `GetVersion(...)`,
    // turn into construct call.
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = {info[0]};
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(_constructor);
    info.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

GetVersion::GetVersion() {}

GetVersion::~GetVersion() {}
