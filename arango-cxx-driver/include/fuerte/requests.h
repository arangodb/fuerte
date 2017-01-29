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
#ifndef ARANGO_CXX_DRIVER_REQUESTS
#define ARANGO_CXX_DRIVER_REQUESTS

#include "message.h"

// This file will be a bit messy in the near future
//
// Maybe we will use some kind of mpl library that allows us to use named and
// defaulted arguments (not only at the end of signature) for constructors.
// The way it is now we need too supply too many different make functions.
// At the is probably the best to create request manually and fill in the
// required fields
//
// TODO -- move impl to cpp once done

namespace arangodb { namespace fuerte { inline namespace v1 {

// Helper and Implementation
  std::unique_ptr<Request> inline
  createRequest(MessageHeader&& messageHeader
               ,mapss&& headerStrings
               ,RestVerb const& verb
               ,ContentType const& contentType
               ){

    auto request = std::unique_ptr<Request>(new Request(std::move(messageHeader),std::move(headerStrings)));

    request->header.restVerb = verb;
    if (!request->header.type){
      request->header.type = MessageType::Request;
    }

    if (!request->header.contentType){
      request->header.contentType = ContentType::VPack;
    }

    return request;
  }

  std::unique_ptr<Request> inline
  createRequest(MessageHeader const& messageHeader
               ,mapss const& headerStrings
               ,std::string const& database
               ,RestVerb const& verb
               ,ContentType const& contentType
               ){
    MessageHeader header = messageHeader;
    mapss strings = headerStrings;
    return createRequest(std::move(header), std::move(strings), database, verb, contentType);
  }

  std::unique_ptr<Request> inline
  createRequest(RestVerb const& verb
               ,ContentType const& contentType
               ){
    return createRequest(MessageHeader(), mapss(), verb, contentType);
  }

// For User
  std::unique_ptr<Request> inline
  createRequest(RestVerb verb
               ,std::string const& path
               ,mapss const& parameter
               ,VBuffer&& payload)
  {
    auto request = createRequest(verb, ContentType::VPack);
    request->header.path = path;
    request->header.parameter = parameter;
    request->addVPack(std::move(payload));
    return request;
  }

  std::unique_ptr<Request> inline
  createRequest(RestVerb verb
               ,std::string const& path
               ,mapss const& parameter
               ,VSlice const& payload)
  {
    auto request = createRequest(verb, ContentType::VPack);
    request->header.path = path;
    request->header.parameter = parameter;
    request->addVPack(payload);
    return request;
  }

  std::unique_ptr<Request> inline
  createRequest(RestVerb verb
               ,std::string const& path
               ,mapss const& parameter = mapss()
               )
  {
    auto request = createRequest(verb, ContentType::VPack);
    request->header.path = path;
    request->header.parameter = parameter;
    return request;
  }
}}}
#endif
