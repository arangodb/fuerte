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
  if(unwrapSelf<NRequest>(info)->_cppClass){
    info.GetReturnValue().Set(Nan::True());
  } else {
    info.GetReturnValue().Set(Nan::False());
  }
}

NAN_METHOD(NRequest::addVPack){ //slice
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  // check type TODO
  uint8_t* data = reinterpret_cast<uint8_t*>(::node::Buffer::Data(info[0])); /// aaaaijajaiai
  std::size_t length = ::node::Buffer::Length(info[0]);

  unwrapSelf<NRequest>(info)->_cppClass->addVPack(fu::VSlice(data));
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NRequest::addBinary){
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  // check type TODO
  uint8_t* data = reinterpret_cast<uint8_t*>(::node::Buffer::Data(info[0])); /// aaaaijajaiai
  std::size_t length = ::node::Buffer::Length(info[0]);

  unwrapSelf<NRequest>(info)->_cppClass->addBinary(data,length);
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NRequest::setPath){
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  unwrapSelf<NRequest>(info)->_cppClass->header.path = to<std::string>(info[0]);
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NRequest::setDatabase){
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  unwrapSelf<NRequest>(info)->_cppClass->header.database = to<std::string>(info[0]);
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NRequest::setRestVerb){
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
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
  else if(restString == "put"){
    verb = fu::RestVerb::Put;
  }
  else if(restString == "delete"){
    verb = fu::RestVerb::Delete;
  } else {
    Nan::ThrowTypeError("invalid rest parameter get/put/post/put/delete are supported");
  }

  unwrapSelf<NRequest>(info)->_cppClass->header.restVerb = verb;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NRequest::setPassword){
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  unwrapSelf<NRequest>(info)->_cppClass->header.password = to<std::string>(info[0]);
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NRequest::setUser){
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  unwrapSelf<NRequest>(info)->_cppClass->header.user = to<std::string>(info[0]);
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NRequest::setContentType){
  throw std::logic_error("implement me");
}

NAN_METHOD(NRequest::addParameter){
  if (info.Length() != 2 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  boost::optional<fu::mapss>& params = unwrapSelf<NRequest>(info)->_cppClass->header.parameter;
  if(!params){
    params = fu::mapss();
  }
  params.get().insert({to<std::string>(info[0]),to<std::string>(info[1])});
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NRequest::addMeta){
  if (info.Length() != 2 ) {
    Nan::ThrowTypeError("Wrong number of Arguments");
  }
  boost::optional<fu::mapss>& params = unwrapSelf<NRequest>(info)->_cppClass->header.meta;
  if(!params){
    params = fu::mapss();
  }
  params.get().insert({to<std::string>(info[0]),to<std::string>(info[1])});
  info.GetReturnValue().Set(info.This());
}



// NResponse
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
  if(unwrapSelf<NResponse>(info)->_cppClass){
    info.GetReturnValue().Set(Nan::True());
  } else {
    info.GetReturnValue().Set(Nan::False());
  }
}

NAN_METHOD(NResponse::buffers){ //slice
  v8::Local<v8::Array> array;
  if (unwrapSelf<NResponse>(info)->_cppClass) {
    fu::Response& res = *unwrapSelf<NResponse>(info)->_cppClass;
    for(auto const& slice : res.slices()){
      Nan::MaybeLocal<v8::Object> buf = Nan::CopyBuffer(
          slice.startAs<char>(), slice.byteSize());
      array->Set(0,buf.ToLocalChecked());
    }
  }
  info.GetReturnValue().Set(array);
}

NAN_METHOD(NResponse::payload){
  if (unwrapSelf<NResponse>(info)->_cppClass) {
    fu::Response& res = *unwrapSelf<NResponse>(info)->_cppClass;
    auto payload = res.payload();
    Nan::MaybeLocal<v8::Object> buf = Nan::CopyBuffer(
        reinterpret_cast<char const *>(payload.first), payload.second);
    info.GetReturnValue().Set(buf.ToLocalChecked());
  }
}

NAN_METHOD(NResponse::getResponseCode){
  if (unwrapSelf<NResponse>(info)->_cppClass) {
    uint32_t rv = unwrapSelf<NResponse>(info)->_cppClass->header.responseCode.get();
    info.GetReturnValue().Set(Nan::New<v8::Uint32>(rv));
  }
}


NAN_METHOD(NResponse::getContentType){
  auto rv = unwrapSelf<NResponse>(info)->_cppClass->header.contentType;
  if(rv){
    info.GetReturnValue().Set(Nan::New(fu::to_string(rv.get())).ToLocalChecked());
  } else {
    info.GetReturnValue().Set(Nan::New(std::string("vpack")).ToLocalChecked());
  }
}

}}}
