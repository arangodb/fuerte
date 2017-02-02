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
#pragma once

// debian sid
#include <nodejs/deps/v8/include/v8.h>
#include <nodejs/src/node.h>
#include <nan.h>

#include <fuerte/fuerte.h>

namespace fu = ::arangodb::fuerte;

namespace arangodb { namespace fuerte { namespace js {

//isOneOf
template<class T, class ...XXS>
struct isOneOf;

template<class T>
struct isOneOf<T> : std::false_type {};

template<class T, class ...XS>
struct isOneOf<T, T, XS...> : std::true_type {};

template<class T, class X, class ...XS>
struct isOneOf<T, X, XS...> : isOneOf<T, XS...> {};

template<class T, class ...XXS>
using isOneOf_t = typename isOneOf<T, XXS...>::type;

template<class T, class ...XXS>
using isOneOf_v = typename isOneOf<T, XXS...>::value;


inline std::string to_string(v8::Local<v8::Value> const& from){
  try {
    if(!from->IsString()){
      throw std::invalid_argument("argument is not a string");
    }
    ::Nan::Utf8String nString(from);
    return std::string(*nString, nString.length());
  }
  catch(std::exception const& e){
    return e.what();
  }
}


template <typename T
         ,typename std::enable_if<isOneOf<T,bool,int,int32_t,uint32_t,uint64_t
                                         >::value
                                 ,int
                                 >::type = 1
         >
T to(v8::Local<v8::Value> const& from){
  return ::Nan::To<T>(from).FromJust();
}

template <typename T
         ,typename std::enable_if<isOneOf_t<T,std::string
                                           >::value
                                 ,int
                                 >::type = 2
         >
T to(v8::Local<v8::Value> const& from){
  return to_string(from);
}

template <typename T
         ,typename std::enable_if<isOneOf_t<T, v8::Boolean, v8::Int32, v8::Integer, v8::Object, v8::Number, v8::String, v8::Uint32
                                           >::value
                                 ,int
                                 >::type = 3
         >
T to(v8::Local<v8::Value> const& from){
  throw std::logic_error("implment me!");
  return  T();
}

template <typename ClassType>
ClassType* unwrap(v8::Local<v8::Value> const& info){
  assert(info->IsObject());
  return Nan::ObjectWrap::Unwrap<ClassType>(info->ToObject());
}

template <typename ClassType, typename T>
ClassType* unwrapSelf(Nan::FunctionCallbackInfo<T> const& info){
  return Nan::ObjectWrap::Unwrap<ClassType>(info.Holder());
}

}}}
