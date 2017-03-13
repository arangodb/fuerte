////////////////////////////////////////////////////////////////////////////////
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
/// @author Jan Christoph Uhde
////////////////////////////////////////////////////////////////////////////////

#include "node_connection.h"
#include "node_request.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <atomic>
#include <fuerte/FuerteLogger.h>
#include <fuerte/helper.h>

namespace arangodb { namespace fuerte { namespace js {


///////////////////////////////////////////////////////////////////////////////
// NConnectionBuilder /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
NAN_METHOD(NConnectionBuilder::New) {
  try {
    if (info.IsConstructCall()) {
      NConnectionBuilder* obj = new NConnectionBuilder();
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {
      v8::Local<v8::Function> builder = Nan::New(constructor());
      info.GetReturnValue().Set(Nan::NewInstance(builder).ToLocalChecked());
    }
 } catch(std::exception const& e){
   Nan::ThrowError("ConnectionBuilder.New binding failed with exception");
 }
}

NAN_METHOD(NConnectionBuilder::connect){
  try {
    v8::Local<v8::Function> connFunction = Nan::New(NConnection::constructor());
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = {info.This()};
    v8::Local<v8::Object> connInstance;
    bool ok = Nan::NewInstance(connFunction, argc, argv).ToLocal(&connInstance);
    if(!ok){
      Nan::ThrowError("ConnectionBuilder.connect binding failed with exception"
                      " - please check connection parameters and server");
      return;
    }
    info.GetReturnValue().Set(connInstance);
  } catch(std::exception const& e){
    std::cerr << "## DRIVER LEVEL EXCEPTION - START ##" << std::endl;
    std::cerr << e.what() << std::endl;
    std::cerr << "## DRIVER LEVEL EXCEPTION - END   ##" << std::endl;
    Nan::ThrowError("ConnectionBuilder.connect binding failed with exception"
                    " - Make sure the server is up and running");
  }
}

NAN_METHOD(NConnectionBuilder::host){
  try {
    if (info.Length() != 1 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
    }
    unwrapSelf<NConnectionBuilder>(info)->_cppClass.host(to<std::string>(info[0]));
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    std::cerr << "## DRIVER LEVEL EXCEPTION - START ##" << std::endl;
    std::cerr << e.what() << std::endl;
    std::cerr << "## DRIVER LEVEL EXCEPTION - END   ##" << std::endl;
    std::string errorMesasge = std::string("ConnectionBuilder.host binding failed with exception: ") + e.what();
    Nan::ThrowError(errorMesasge.c_str());
  }
}

NAN_METHOD(NConnectionBuilder::async){
  try {
    if (info.Length() != 1 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
    }
    unwrapSelf<NConnectionBuilder>(info)->_cppClass.async(to<bool>(info[0]));
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("ConnectionBuilder.async binding failed with exception");
  }
}

NAN_METHOD(NConnectionBuilder::user){
  try {
    if (info.Length() != 1 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
    }
    unwrapSelf<NConnectionBuilder>(info)->_cppClass.user(to<std::string>(info[0]));
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("ConnectionBuilder.user binding failed with exception");
  }
}

NAN_METHOD(NConnectionBuilder::password){
  try {
    if (info.Length() != 1 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
    }
    unwrapSelf<NConnectionBuilder>(info)->_cppClass.password(to<std::string>(info[0]));
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("ConnectionBuilder.user binding failed with exception");
  }
}


///////////////////////////////////////////////////////////////////////////////
// NConnection ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
NAN_METHOD(NConnection::New) {
  try {
    if (info.IsConstructCall()) {
      NConnection* obj = new NConnection();
      if(info[0]->IsObject()){ // NConnectionBuilderObject -- exact type check?
        obj->_cppClass = unwrap<NConnectionBuilder>(info[0])->_cppClass.connect();
        if(obj->_cppClass == nullptr){
          Nan::ThrowError("Connection.New binding failed with exception - check connetction string");
          return;
        }
      }
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {
      v8::Local<v8::Function> cons = Nan::New(constructor());
      info.GetReturnValue().Set(Nan::NewInstance(cons).ToLocalChecked());
    }
  } catch(std::exception const& e){
    Nan::ThrowError("Connection.New binding failed with exception");
  }
}

NAN_METHOD(NConnection::reqestsLeft) {
  try {
    auto& cls = unwrapSelf<NConnection>(info)->_cppClass;
    info.GetReturnValue().Set(Nan::New<v8::Integer>(cls->requestsLeft()));
  } catch(std::exception const& e){
    Nan::ThrowError("Connection.reqestsLeft binding failed with exception");
  }
}

///////////////////////////////////////////////////////////////////////////////
// SendRequest ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// When we switch to c++14 we should use UniquePersistent and move it into
// the generalized lambda caputre avoiding the locking alltogehter
static std::map<fu::MessageID
               ,std::pair<v8::Persistent<v8::Function> // error
                         ,v8::Persistent<v8::Function> // success
                         >
              > callbackMap;

static std::mutex maplock;
static std::atomic<uint64_t> jsMessageID(0);

const std::string callbackErrorMessage(
  "##################################################\n"
  "#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#\n"
  "#@Failed to create request instance in callback.@#\n"
  "#@This could be an error in you JavaScript Code!@#\n"
  "#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#\n"
  "##################################################\n"
);

NAN_METHOD(NConnection::sendRequest) {
  try {
    if (info.Length() != 3 ) {
      Nan::ThrowTypeError("Not 3 Arguments");
    }

    if (!info[1]->IsFunction() || !info[2]->IsFunction()){
      Nan::ThrowTypeError("Callback is not a Function");
    }

    // get isolate - has node only one context??!?!? how do they work
    // context is probably a lighter version of isolate but does not allow threads
    v8::Isolate* iso = v8::Isolate::GetCurrent();

    uint64_t id = jsMessageID.fetch_add(1);
    {
      std::lock_guard<std::mutex> lock(maplock);
      auto& callbackPair = callbackMap[id]; //create map element
      auto jsOnErr = v8::Local<v8::Function>::Cast(info[1]);
      callbackPair.first.Reset(iso, jsOnErr);

      auto jsOnSucc = v8::Local<v8::Function>::Cast(info[2]);
      callbackPair.second.Reset(iso, jsOnSucc);
    }

    fu::OnErrorCallback err = [iso,id](unsigned err
                                ,std::unique_ptr<fu::Request> creq
                                ,std::unique_ptr<fu::Response> cres)
    {
      try {
#if ENABLE_FUERTE_LOG_NODE > 0
        if(creq->header.responseCode && creq->header.responseCode.get() >= 400){
          std::cout << "response code >= 400\n"
                    << fu::to_string(*creq)
                    << fu::to_string(*cres)
                    << std::endl;
        }
#endif
        v8::HandleScope scope(iso);
        auto jsOnErr = v8::Local<v8::Function>();
        { //create local function and dispose persistent
          std::lock_guard<std::mutex> lock(maplock);
          auto& mapElement = callbackMap[id];
          jsOnErr = v8::Local<v8::Function>::New(iso,mapElement.first);
          mapElement.first.Reset();
          mapElement.second.Reset();
          callbackMap.erase(id);
        }

        // wrap request
        v8::Local<v8::Function> requestProto = v8::Local<v8::Function>::New(iso,NRequest::constructor());
        assert(!requestProto.IsEmpty());
        v8::Local<v8::Object> reqObj;
        bool ok = requestProto->NewInstance(iso->GetCurrentContext()).ToLocal(&reqObj);
        if(!ok) {
          std::cerr << callbackErrorMessage << std::endl;
          iso->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(iso,callbackErrorMessage.c_str())));
          return;
        }
        unwrap<NRequest>(reqObj)->setCppClass(std::move(creq));

        // wrap response
        v8::Local<v8::Function> responseProto = v8::Local<v8::Function>::New(iso,NResponse::constructor());
        assert(!responseProto.IsEmpty());
        v8::Local<v8::Object> resObj;
        ok = responseProto->NewInstance(iso->GetCurrentContext()).ToLocal(&resObj);
        if(!ok) {
          std::cerr << callbackErrorMessage << std::endl;
          iso->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(iso,callbackErrorMessage.c_str())));
          return;
        }
        unwrap<NResponse>(resObj)->setCppClass(std::move(cres));

        // build args
        const unsigned argc = 3;
        v8::Local<v8::Value> argv[argc] = { Nan::New<v8::Integer>(err), reqObj, resObj };

        // call
        jsOnErr->Call(iso->GetCurrentContext(), jsOnErr, argc, argv);
      } catch (std::exception const& e){
        std::string errorMesasge("Exception in success callback: ");
        errorMesasge += e.what();
        iso->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(iso,errorMesasge.c_str())));
      }
    };

    fu::OnSuccessCallback succ = [iso,id](std::unique_ptr<fu::Request> creq
                                   ,std::unique_ptr<fu::Response> cres)
    {
      try {
#if ENABLE_FUERTE_LOG_NODE > 0
        if(creq->header.responseCode && creq->header.responseCode.get() >= 400){
          std::cout << "response code >= 400\n"
                    << fu::to_string(*creq)
                    << fu::to_string(*cres)
                    << std::endl;
        }
#endif
        FUERTE_LOG_NODE << "enter on fuerte-node success callback" << std::endl;
        v8::HandleScope scope(iso);

        auto jsOnSucc = v8::Local<v8::Function>();
        { // create locacl function and dispose persistent
          std::lock_guard<std::mutex> lock(maplock);
          auto& mapElement = callbackMap[id];
          //create local function
          jsOnSucc = v8::Local<v8::Function>::New(iso,callbackMap[id].second);

          //dispose map element
          mapElement.first.Reset(); // do not depend on kResetInDestructorFlag of Persistent
          mapElement.second.Reset();
          callbackMap.erase(id);
        }

            // other ways to do the same
            // v8::Local<v8::Function> requestProto = Nan::New(NConnection::constructor()); // with Nan
            // auto reqObj = Nan::NewInstance(requestProto).ToLocalChecked(); // with Nan
            // auto reqObj = requestProto->NewInstance(iso->GetCurrentContext()).ToLocalChecked(); // should not crash!

        // wrap request
        v8::Local<v8::Function> requestProto = v8::Local<v8::Function>::New(iso,NRequest::constructor());
        assert(!requestProto.IsEmpty());
        v8::Local<v8::Object> reqObj;
        bool ok = requestProto->NewInstance(iso->GetCurrentContext()).ToLocal(&reqObj);
        if(!ok) {
          std::cerr << callbackErrorMessage << std::endl;
          iso->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(iso,callbackErrorMessage.c_str())));
          return;
        }
        unwrap<NRequest>(reqObj)->setCppClass(std::move(creq));

        // wrap response
        v8::Local<v8::Function> responseProto = v8::Local<v8::Function>::New(iso,NResponse::constructor());
        assert(!responseProto.IsEmpty());
        v8::Local<v8::Object> resObj;
        ok = responseProto->NewInstance(iso->GetCurrentContext()).ToLocal(&resObj);
        if(!ok) {
          std::cerr << callbackErrorMessage << std::endl;
          iso->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(iso,callbackErrorMessage.c_str())));
          return;
        }
        unwrap<NResponse>(resObj)->setCppClass(std::move(cres));

        // build args
        const unsigned argc = 2;
        v8::Local<v8::Value> argv[argc] = { reqObj, resObj };

        // call
        //jsOnSucc->Call(v8::Null(iso), argc, argv);
        jsOnSucc->Call(iso->GetCurrentContext(), jsOnSucc, argc, argv);
        //jsOnSucc->Call(iso->GetCurrentContext(), iso->GetCurrentContext()->Global(), argc, argv);
      } catch (std::exception const& e){
        std::string errorMesasge("Exception in error callback: ");
        errorMesasge += e.what();
        iso->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(iso,errorMesasge.c_str())));
      }
    };

    //finally invoke the c++ callback
    auto req = std::unique_ptr<fu::Request>(new fu::Request(*(unwrap<NRequest>(info[0])->_cppClass))); //copy request so the old object stays valid
    unwrapSelf<NConnection>(info)->_cppClass->sendRequest(std::move(req), err, succ);
  } catch(std::exception const& e){
    Nan::ThrowError("Connection.sendRequest binding failed with exception");
    return;
  }
  FUERTE_LOG_NODE << "exit on fuerte-node success callback" << std::endl;
}

}}}
