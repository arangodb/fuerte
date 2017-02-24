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
#ifndef ARANGO_CXX_DRIVER_MESSAGE
#define ARANGO_CXX_DRIVER_MESSAGE

#include "types.h"

#include <boost/optional.hpp>
#include <string>
#include <vector>
#include <map>

namespace arangodb { namespace fuerte { inline namespace v1 {

namespace vst {
  class VstConnection;
}

const std::string fu_content_type_key("content-type");

// mabye get rid of optional
struct MessageHeader {
  MessageHeader(MessageHeader const&) = default;
  MessageHeader() = default;
  MessageHeader(MessageHeader&&) = default;

  ::boost::optional<int> version;
  ::boost::optional<MessageType> type;
  ::boost::optional<uint32_t> responseCode;
  ::boost::optional<std::string> database;
  ::boost::optional<RestVerb> restVerb;
  ::boost::optional<std::string> path;
  ::boost::optional<mapss> parameter;
  ::boost::optional<mapss> meta;
  ::boost::optional<std::string> user;
  ::boost::optional<std::string> password;
  ::boost::optional<std::size_t> byteSize; //for debugging

  //get set content type
  ContentType contentType() const {
    if(!meta){
      return ContentType::Unset;
    }

    auto& hmap = meta.get();
    auto found =  hmap.find(fu_content_type_key);
    if(found == hmap.end()){
      return ContentType::Unset;
    }

    return to_ContentType(found->second);
  }

  void contentType(std::string const& type){
    if(!meta){
      meta = mapss();
    }

    meta.get()[fu_content_type_key] = type;
  }

  void contentType(ContentType type){
    contentType(to_string(type));
  }
};

std::string to_string(MessageHeader const&);

// create a map<string,string> from header object
mapss headerStrings(MessageHeader const& header);

// TODO SPLIT MESSAGE INTO REQEST / RESPONSE
class Message {
friend class VstConnection;
public:
  Message(MessageHeader&& messageHeader = MessageHeader()
         ,mapss&& headerStrings = mapss()
         )
         :header(std::move(messageHeader))
         ,_sealed(false)
         ,_modified(true)
         ,_isVpack(boost::none)
         ,_builder(nullptr)
         ,_payloadLength(0)
         {
           if (!headerStrings.empty()){
            header.meta = std::move(headerStrings);
           }
         }

  Message(MessageHeader const& messageHeader
         ,mapss const& headerStrings
         )
         :header(messageHeader)
         ,_sealed(false)
         ,_modified(true)
         ,_isVpack(boost::none)
         ,_builder(nullptr)
         ,_payloadLength(0)
         {
           if (!headerStrings.empty()){
            header.meta = std::move(headerStrings);
           }
         }

  MessageHeader header;
  uint64_t messageid; //generate by some singleton

  ///////////////////////////////////////////////
  // add payload
  ///////////////////////////////////////////////
  void addVPack(VSlice const& slice);
  void addVPack(VBuffer const& buffer);
  void addVPack(VBuffer&& buffer);
  void addBinary(uint8_t const* data, std::size_t length);
  void addBinarySingle(VBuffer&& buffer);

  ///////////////////////////////////////////////
  // get payload
  ///////////////////////////////////////////////
  std::vector<VSlice>const & slices();
  std::pair<uint8_t const *, std::size_t> payload(); //as binary

  ContentType contentType() const {
    return header.contentType();
  }
  void contentType(std::string const& type){
    header.contentType(type);
  }
  void contentType(ContentType type){
    header.contentType(type);
  }

private:
  VBuffer _payload;
  bool _sealed;
  bool _modified;
  ::boost::optional<bool> _isVpack;
  std::shared_ptr<VBuilder> _builder;
  std::vector<VSlice> _slices;
  std::size_t _payloadLength; // because VPackBuffer has quirks we need
                              // to track the Length manually
};

class Request : public Message {
public:
  Request(MessageHeader&& messageHeader = MessageHeader()
         ,mapss&& headerStrings = mapss()
         ): Message(std::move(messageHeader), std::move(headerStrings))
         {
           header.type = MessageType::Request;
         }
  Request(MessageHeader const& messageHeader
         ,mapss const& headerStrings
         ): Message(messageHeader, headerStrings)
         {
           header.type = MessageType::Request;
         }

};

class Response : public Message {
public:
  Response(MessageHeader&& messageHeader = MessageHeader()
          ,mapss&& headerStrings = mapss()
          )
          :Message(std::move(messageHeader), std::move(headerStrings))
          {
            header.type = MessageType::Response;
          }


};

//std::unique_ptr<Response> createResponse(unsigned code);

}}}
#endif
