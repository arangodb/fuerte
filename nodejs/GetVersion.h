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
#ifndef GETVERSION_H
#define GETVERSION_H

#include <nan.h>
#include <v8.h>
#include <curlpp/Easy.hpp>
#include <curlpp/Multi.hpp>

#include "Buffer.h"

//
// GetVersion class
//
// This is the object used by Javascript to get the ArangoDB
// version
//
class GetVersion : public Nan::ObjectWrap
{
  public:
    static void Init(v8::Local<v8::Object> exports);

    GetVersion();
    ~GetVersion();

  private:
    std::string result();
    std::string version();
    bool process();
    bool config();
    void checkMsgs();
    void errorMessage(const std::string &inp);
    static void New(
      const Nan::FunctionCallbackInfo<v8::Value> &info);
    static void Result(
      const Nan::FunctionCallbackInfo<v8::Value> &info);
    static void Version(
      const Nan::FunctionCallbackInfo<v8::Value> &info);
    static void Process(
      const Nan::FunctionCallbackInfo<v8::Value> &info);
    static void Config(
      const Nan::FunctionCallbackInfo<v8::Value> &info);
    Buffer _buf;
    curlpp::Easy _easy;
    curlpp::Multi _multi;
    static Nan::Persistent<v8::Function> _constructor;
};

inline std::string GetVersion::result()
{
  return _buf.result();
}


#endif  // GETVERSION_H
