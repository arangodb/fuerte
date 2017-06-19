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
/// @author Ewout Prangsma
////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef ARANGO_CXX_DRIVER_MESSAGE
#define ARANGO_CXX_DRIVER_MESSAGE

#include "types.h"

#include <boost/optional.hpp>
#include <boost/asio/buffer.hpp>
#include <string>
#include <vector>
#include <map>

namespace arangodb { namespace fuerte { inline namespace v1 {

const std::string fu_content_type_key("content-type");
const std::string fu_accept_key("accept");

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

  // content type accessors
  std::string contentTypeString() const;
  ContentType contentType() const;
  void contentType(std::string const& type);
  void contentType(ContentType type);

  // accept header accessors
  std::string acceptTypeString() const;
  ContentType acceptType() const;
  void acceptType(std::string const& type);
  void acceptType(ContentType type);
};

std::string to_string(MessageHeader const&);

// create a map<string,string> from header object
mapss headerStrings(MessageHeader const& header);

// Message is base class for message being send to (Request) or
// from (Response) a server.
class Message {
protected:
  Message(MessageHeader&& messageHeader = MessageHeader(), mapss&& headerStrings = mapss())
    : header(std::move(messageHeader)),
      messageID(123456789)
         {
           if (!headerStrings.empty()){
            header.meta = std::move(headerStrings);
           }
         }

  Message(MessageHeader const& messageHeader, mapss const& headerStrings)
    : header(messageHeader),
      messageID(123456789)
         {
           if (!headerStrings.empty()){
            header.meta = std::move(headerStrings);
           }
         }

public:
  MessageHeader header;
  MessageID messageID; //generate by some singleton

  ///////////////////////////////////////////////
  // get payload
  ///////////////////////////////////////////////
  virtual std::vector<VSlice>const & slices() = 0;
  virtual boost::asio::const_buffer payload() const = 0; 
  std::string payloadAsString() const {
    auto p = payload();
    return std::string(boost::asio::buffer_cast<char const*>(p), boost::asio::buffer_size(p));
  }

  // content-type header accessors
  std::string contentTypeString() const;
  ContentType contentType() const;

  // accept header accessors
  std::string acceptTypeString() const;
  ContentType acceptType() const;
};

// Request contains the message send to a server in a request.
class Request : public Message {
public:
  Request(MessageHeader&& messageHeader = MessageHeader(), mapss&& headerStrings = mapss())
    : Message(std::move(messageHeader), std::move(headerStrings)),
      _sealed(false),
      _modified(true),
      _isVpack(boost::none),
      _builder(nullptr),
      _payloadLength(0)
         {
           header.type = MessageType::Request;
         }
  Request(MessageHeader const& messageHeader, mapss const& headerStrings)
    : Message(messageHeader, headerStrings),
      _sealed(false),
      _modified(true),
      _isVpack(boost::none),
      _builder(nullptr),
      _payloadLength(0)
         {
           header.type = MessageType::Request;
         }

  ///////////////////////////////////////////////
  // add payload
  ///////////////////////////////////////////////
  void addVPack(VSlice const& slice);
  void addVPack(VBuffer const& buffer);
  void addVPack(VBuffer&& buffer);
  void addBinary(uint8_t const* data, std::size_t length);
  void addBinarySingle(VBuffer&& buffer);

  // content-type header accessors
  void contentType(std::string const& type);
  void contentType(ContentType type);

  // accept header accessors
  void acceptType(std::string const& type);
  void acceptType(ContentType type);

  ///////////////////////////////////////////////
  // get payload
  ///////////////////////////////////////////////
  virtual std::vector<VSlice>const & slices() override;
  virtual boost::asio::const_buffer payload() const override; 

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

// Response contains the message resulting from a request to a server.
class Response : public Message {
public:
  Response(MessageHeader&& messageHeader = MessageHeader(), mapss&& headerStrings = mapss())
    : Message(std::move(messageHeader), std::move(headerStrings))
          {
            header.type = MessageType::Response;
          }

  ///////////////////////////////////////////////
  // get/set payload
  ///////////////////////////////////////////////
  virtual std::vector<VSlice>const & slices() override;
  virtual boost::asio::const_buffer payload() const override; 

  void setPayload(VBuffer&& buffer, size_t payloadOffset);

private:
  VBuffer _payload;
  size_t _payloadOffset;
  std::vector<VSlice> _slices;
};

}}}
#endif
