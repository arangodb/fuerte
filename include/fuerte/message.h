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

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "types.h"

#include <boost/optional.hpp>
#include <boost/asio/buffer.hpp>

namespace arangodb { namespace fuerte { inline namespace v1 {

const std::string fu_content_type_key("content-type");
const std::string fu_accept_key("accept");

// mabye get rid of optional
struct MessageHeader {
  MessageHeader(MessageHeader const&) = default;
  MessageHeader() = default;
  MessageHeader(MessageHeader&&) = default;

  ::boost::optional<int> version;
  ::boost::optional<MessageType> type;        // Type of message
  ::boost::optional<StatusCode> responseCode; // Response code (response only)
  ::boost::optional<std::string> database;    // Database that is the target of the request
  ::boost::optional<RestVerb> restVerb;       // HTTP method
  ::boost::optional<std::string> path;        // Local path of the request
  ::boost::optional<StringMap> parameters;    // Query parameters
  ::boost::optional<StringMap> meta;          // Header meta data
  ::boost::optional<std::string> encryption;  // Authentication: encryption field
  ::boost::optional<std::string> user;        // Authentication: username
  ::boost::optional<std::string> password;    // Authentication: password
  ::boost::optional<std::string> token;       // Authentication: JWT token
  ::boost::optional<std::size_t> byteSize;    // for debugging

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

  // query parameter helpers 
  void addParameter(std::string const& key, std::string const& value);
  // Header metadata helpers 
  void addMeta(std::string const& key, std::string const& value);
  // Get value for header metadata key, returns empty string if not found.
  std::string metaByKey(std::string const& key) const;
};

std::string to_string(MessageHeader const&);

// create a map<string,string> from header object
StringMap headerStrings(MessageHeader const& header);

// Message is base class for message being send to (Request) or
// from (Response) a server.
class Message {
protected:
  Message(MessageHeader&& messageHeader = MessageHeader(), StringMap&& headerStrings = StringMap())
    : header(std::move(messageHeader)),
      messageID(123456789)
         {
           if (!headerStrings.empty()){
            header.meta = std::move(headerStrings);
           }
         }

  Message(MessageHeader const& messageHeader, StringMap const& headerStrings)
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
  static std::chrono::milliseconds _defaultTimeout;
public:
  Request(MessageHeader&& messageHeader = MessageHeader(), StringMap&& headerStrings = StringMap())
    : Message(std::move(messageHeader), std::move(headerStrings)),
      _sealed(false),
      _modified(true),
      _isVpack(boost::none),
      _builder(nullptr),
      _payloadLength(0),
      _timeout(std::chrono::duration_cast<std::chrono::milliseconds>(_defaultTimeout))
         {
           header.type = MessageType::Request;
         }
  Request(MessageHeader const& messageHeader, StringMap const& headerStrings)
    : Message(messageHeader, headerStrings),
      _sealed(false),
      _modified(true),
      _isVpack(boost::none),
      _builder(nullptr),
      _payloadLength(0),
      _timeout(std::chrono::duration_cast<std::chrono::milliseconds>(_defaultTimeout))
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

  // get timeout 
  inline std::chrono::milliseconds timeout() const { return _timeout; }
  // set timeout 
  void timeout(std::chrono::milliseconds timeout) { _timeout = timeout; }

private:
  VBuffer _payload;
  bool _sealed;
  bool _modified;
  ::boost::optional<bool> _isVpack;
  std::shared_ptr<VBuilder> _builder;
  std::vector<VSlice> _slices;
  std::size_t _payloadLength; // because VPackBuffer has quirks we need
                              // to track the Length manually
  std::chrono::milliseconds _timeout;
};

// Response contains the message resulting from a request to a server.
class Response : public Message {
public:
  Response(MessageHeader&& messageHeader = MessageHeader(), StringMap&& headerStrings = StringMap())
    : Message(std::move(messageHeader), std::move(headerStrings))
          {
            header.type = MessageType::Response;
          }

  ///////////////////////////////////////////////
  // get / check status
  ///////////////////////////////////////////////

  // statusCode returns the (HTTP) status code for the request (400==OK).
  StatusCode statusCode() { return header.responseCode ? header.responseCode.get() : 0; }
  // checkStatus returns true if the statusCode equals one of the given valid code, false otherwise.
  bool checkStatus(std::initializer_list<StatusCode> validStatusCodes) {
    auto actual = statusCode();
    for (auto code : validStatusCodes) {
      if (code == actual) return true;
    }
    return false;
  }
  // assertStatus throw an exception if the statusCode does not equal one of the given valid codes.
  void assertStatus(std::initializer_list<StatusCode> validStatusCodes) {
    if (!checkStatus(validStatusCodes)) {
      throw std::runtime_error("invalid status " + std::to_string(statusCode()));
    }
  }

  ///////////////////////////////////////////////
  // get/set payload
  ///////////////////////////////////////////////
  bool isContentTypeJSON() const;
  bool isContentTypeVPack() const;
  bool isContentTypeHtml() const;
  bool isContentTypeText() const;
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
