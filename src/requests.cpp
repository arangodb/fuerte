////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2017 ArangoDB GmbH, Cologne, Germany
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

#include <fuerte/requests.h>

namespace arangodb { namespace fuerte { inline namespace v1 {

// Helper and Implementation
std::unique_ptr<Request>
createRequest(MessageHeader&& messageHeader
             ,StringMap&& headerStrings
             ,RestVerb const& verb
             ,ContentType const& contentType
             ){

  auto request = std::unique_ptr<Request>(new Request(std::move(messageHeader),std::move(headerStrings)));

  request->header.restVerb = verb;
  if (!request->header.type){
    request->header.type = MessageType::Request;
  }

  request->header.contentType(contentType);
  // fuerte requests defualt to vpack content type for accept
  request->header.acceptType(ContentType::VPack);

  return request;
}

std::unique_ptr<Request>
createRequest(MessageHeader const& messageHeader
             ,StringMap const& headerStrings
             ,std::string const& database
             ,RestVerb const& verb
             ,ContentType const& contentType
             ){
  MessageHeader header = messageHeader;
  StringMap strings = headerStrings;
  return createRequest(std::move(header), std::move(strings), database, verb, contentType);
}

std::unique_ptr<Request>
createRequest(RestVerb const& verb
             ,ContentType const& contentType
             ){
  return createRequest(MessageHeader(), StringMap(), verb, contentType);
}

// For User
std::unique_ptr<Request>
createRequest(RestVerb verb
             ,std::string const& path
             ,StringMap const& parameters
             ,VBuffer&& payload)
{
  auto request = createRequest(verb, ContentType::VPack);
  request->header.path = path;
  request->header.parameters = parameters;
  request->addVPack(std::move(payload));
  return request;
}

std::unique_ptr<Request>
createRequest(RestVerb verb
             ,std::string const& path
             ,StringMap const& parameters
             ,VSlice const& payload)
{
  auto request = createRequest(verb, ContentType::VPack);
  request->header.path = path;
  request->header.parameters = parameters;
  request->addVPack(payload);
  return request;
}

std::unique_ptr<Request>
createRequest(RestVerb verb
             ,std::string const& path
             ,StringMap const& parameters
             )
{
  auto request = createRequest(verb, ContentType::VPack);
  request->header.path = path;
  request->header.parameters = parameters;
  return request;
}

}}}
