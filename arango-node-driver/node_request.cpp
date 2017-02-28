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

#include "node_request.h"
#include <iostream>
#include <memory>

namespace arangodb { namespace fuerte { namespace js {

//mssing slice, pack, restverb


// NRequest
NAN_METHOD(NRequest::New) {
  if (info.IsConstructCall()) {
      NRequest* obj = new NRequest();
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
  } else {
    v8::Local<v8::Function> cons = Nan::New(constructor());
    info.GetReturnValue().Set(Nan::NewInstance(cons).ToLocalChecked());
 }
}

NAN_METHOD(NRequest::notNull) {
  try {
    if(unwrapSelf<NRequest>(info)->_cppClass){
      info.GetReturnValue().Set(Nan::True());
    } else {
      info.GetReturnValue().Set(Nan::False());
    }
  } catch(std::exception const& e){
    Nan::ThrowError("Request.notNull binding failed with exception");
  }
}

NAN_METHOD(NRequest::addVPack){ //slice
  try {
    if (info.Length() != 1 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
      return;
    }
    // check type TODO
    uint8_t* data = reinterpret_cast<uint8_t*>(::node::Buffer::Data(info[0])); /// aaaaijajaiai
    std::size_t length = ::node::Buffer::Length(info[0]);

    unwrapSelf<NRequest>(info)->_cppClass->addVPack(fu::VSlice(data));
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("Request.addVPack binding failed with exception");
  }
}

NAN_METHOD(NRequest::addBinary){
  try {
    if (info.Length() != 1 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
      return;
    }
    // check type TODO
    uint8_t* data = reinterpret_cast<uint8_t*>(::node::Buffer::Data(info[0])); /// aaaaijajaiai
    std::size_t length = ::node::Buffer::Length(info[0]);

    unwrapSelf<NRequest>(info)->_cppClass->addBinary(data,length);
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("Request.addBinary binding failed with exception");
  }
}

NAN_METHOD(NRequest::setPath){
  try {
    if (info.Length() != 1 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
      return;
    }
    unwrapSelf<NRequest>(info)->_cppClass->header.path = to<std::string>(info[0]);
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("Request.setPath binding failed with exception");
  }
}

NAN_METHOD(NRequest::setDatabase){
  try {
    if (info.Length() != 1 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
      return;
    }
    unwrapSelf<NRequest>(info)->_cppClass->header.database = to<std::string>(info[0]);
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("Request.setDatabase binding failed with exception");
  }
}

NAN_METHOD(NRequest::setRestVerb){
  try {
    if (info.Length() != 1 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
      return;
    }
    std::string restString = to<std::string>(info[0]);

    fu::RestVerb verb = fu::RestVerb::Illegal;

    if(restString == "get"){
      verb = fu::RestVerb::Get;
    }
    else if(restString == "put"){
      verb = fu::RestVerb::Put;
    }
    else if(restString == "post"){
      verb = fu::RestVerb::Post;
    }
    else if(restString == "patch"){
      verb = fu::RestVerb::Patch;
    }
    else if(restString == "delete"){
      verb = fu::RestVerb::Delete;
    } else {
      Nan::ThrowTypeError("invalid rest parameter get/put/post/patch/delete are supported");
      return;
    }

    unwrapSelf<NRequest>(info)->_cppClass->header.restVerb = verb;
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("Request.setRestVerb binding failed with exception");
  }
}

NAN_METHOD(NRequest::setPassword){
  try {
    if (info.Length() != 1 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
      return;
    }
    unwrapSelf<NRequest>(info)->_cppClass->header.password = to<std::string>(info[0]);
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("Request.setPassword binding failed with exception");
  }
}

NAN_METHOD(NRequest::setUser){
  try {
    if (info.Length() != 1 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
      return;
    }
    unwrapSelf<NRequest>(info)->_cppClass->header.user = to<std::string>(info[0]);
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("Request.setUser binding failed with exception");
  }
}

NAN_METHOD(NRequest::setContentType){
  try {
    unwrapSelf<NRequest>(info)->_cppClass->contentType(to<std::string>(info[0]));
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("Request.setUser binding failed with exception");
  }
}

NAN_METHOD(NRequest::setAcceptType){
  try {
    unwrapSelf<NRequest>(info)->_cppClass->acceptType(to<std::string>(info[0]));
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("Request.setUser binding failed with exception");
  }
}

NAN_METHOD(NRequest::addParameter){
  try {
    if (info.Length() != 2 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
      return;
    }
    boost::optional<fu::mapss>& params = unwrapSelf<NRequest>(info)->_cppClass->header.parameter;
    if(!params){
      params = fu::mapss();
    }
    params.get().insert({to<std::string>(info[0]),to<std::string>(info[1])});
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("Request.addParameter binding failed with exception");
  }
}

NAN_METHOD(NRequest::addMeta){
  try {
    if (info.Length() != 2 ) {
      Nan::ThrowTypeError("Wrong number of Arguments");
      return;
    }
    boost::optional<fu::mapss>& params = unwrapSelf<NRequest>(info)->_cppClass->header.meta;
    if(!params){
      params = fu::mapss();
    }
    params.get().insert({to<std::string>(info[0]),to<std::string>(info[1])});
    info.GetReturnValue().Set(info.This());
  } catch(std::exception const& e){
    Nan::ThrowError("Request.addMeta binding failed with exception");
  }
}



// NResponse
const char* response_is_null("C++ Response is nullptr - maybe you did not receive a response - please check the error code!");

NAN_METHOD(NResponse::New) {
  if (info.IsConstructCall()) {
      NResponse* obj = new NResponse();
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
  } else {
    v8::Local<v8::Function> cons = Nan::New(constructor());
    info.GetReturnValue().Set(Nan::NewInstance(cons).ToLocalChecked());
 }
}

NAN_METHOD(NResponse::notNull) {
  try {
    if(unwrapSelf<NResponse>(info)->_cppClass){
      info.GetReturnValue().Set(Nan::True());
    } else {
      info.GetReturnValue().Set(Nan::False());
    }
  } catch(std::exception const& e){
    Nan::ThrowError("Response.notNull binding failed with exception");
  }
}

NAN_METHOD(NResponse::buffers){ //slice
  try {
    if (unwrapSelf<NResponse>(info)->_cppClass) {
      v8::Local<v8::Array> array;
      fu::Response& res = *unwrapSelf<NResponse>(info)->_cppClass;
      for(auto const& slice : res.slices()){
        Nan::MaybeLocal<v8::Object> buf = Nan::CopyBuffer(
            slice.startAs<char>(), slice.byteSize());
        array->Set(0,buf.ToLocalChecked());
      }
      info.GetReturnValue().Set(array);
    } else {
      Nan::ThrowError(response_is_null);
      return;
    }
  } catch(std::exception const& e){
    Nan::ThrowError("Response.buffers binding failed with exception");
  }
}

NAN_METHOD(NResponse::payload){
  try {
    if (unwrapSelf<NResponse>(info)->_cppClass) {
      fu::Response& res = *unwrapSelf<NResponse>(info)->_cppClass;
      auto payload = res.payload();
      // std::cout << std::string(reinterpret_cast<char const*>(payload.first), payload.second) << std::endl;
      Nan::MaybeLocal<v8::Object> buf = Nan::CopyBuffer(
          reinterpret_cast<char const *>(payload.first), payload.second);
      info.GetReturnValue().Set(buf.ToLocalChecked());
    } else {
      Nan::ThrowError(response_is_null);
      return;
    }
  } catch(std::exception const& e){
    Nan::ThrowError("Response.payload binding failed with exception");
  }
}

NAN_METHOD(NResponse::getResponseCode){
  try {
    if (unwrapSelf<NResponse>(info)->_cppClass) {
      uint32_t rv = unwrapSelf<NResponse>(info)->_cppClass->header.responseCode.get();
      info.GetReturnValue().Set(Nan::New<v8::Uint32>(rv));
    } else {
      Nan::ThrowError(response_is_null);
      return;
    }
  } catch(std::exception const& e){
    Nan::ThrowError("Response.getResponseCode binding failed with exception");
  }
}

NAN_METHOD(NResponse::getMeta){
  try{
    fu::Response* cResponse= unwrapSelf<NResponse>(info)->_cppClass.get();
    if (cResponse) {
      auto& header = cResponse->header;
      auto headers = Nan::New<v8::Object>();
      if(header.meta == boost::none){
          //Nan::ThrowError("C++ Response Object ist missing meta in header");
          info.GetReturnValue().Set(headers);
          return;
      }

      fu::mapss meta = header.meta.value();
      for(auto& pair : meta){
        std::string const& key = pair.first;
        std::string const& val = pair.second;
        auto k = Nan::New(key);
        auto v = Nan::New(val);
        auto done = headers->Set(info.GetIsolate()->GetCurrentContext(),k.ToLocalChecked(),v.ToLocalChecked());
        if(!done.FromJust()){
          Nan::ThrowError("Unable to write meta value to v8 object.");
          return;
        }
      }
      info.GetReturnValue().Set(headers);
    } else {
      Nan::ThrowError(response_is_null);
      return;
    }
  } catch(std::exception const& e){
    Nan::ThrowError("Reponse.getMeta binding failed with exception");
  }
}

NAN_METHOD(NResponse::getContentType){
  try{
    auto cppClass = unwrapSelf<NResponse>(info)->_cppClass.get();
    if(cppClass){
      if(cppClass->contentType() != fu::ContentType::Unset){
        info.GetReturnValue().Set(Nan::New(cppClass->contentTypeString()).ToLocalChecked());
      } else {
        info.GetReturnValue().Set(Nan::New(fu::to_string(fu::ContentType::Unset)).ToLocalChecked());
      }
    } else {
      Nan::ThrowError(response_is_null);
      return;
    }
  } catch(std::exception const& e){
    Nan::ThrowError("Response.getConentType binding failed with exception");
  }
}

NAN_METHOD(NResponse::getAcceptType){
  try{
    auto cppClass = unwrapSelf<NResponse>(info)->_cppClass.get();
    if(cppClass){
      if(cppClass->acceptType() != fu::ContentType::Unset){
        info.GetReturnValue().Set(Nan::New(cppClass->acceptTypeString()).ToLocalChecked());
      } else {
        info.GetReturnValue().Set(Nan::New(fu::to_string(fu::ContentType::Unset)).ToLocalChecked());
      }
    } else {
      Nan::ThrowError(response_is_null);
      return;
    }
  } catch(std::exception const& e){
    Nan::ThrowError("Response.getConentType binding failed with exception");
  }
}

}}}
