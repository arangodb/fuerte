#pragma once

#include "types.h"

#include <boost/optional.hpp>
#include <string>
#include <vector>
#include <map>

// TODO - move implementation to cpp

namespace arangodb { namespace fuerte { inline namespace v1 {

namespace vst {
  class VstConnection;
}

// mabye get rid of optional
struct MessageHeader {
  MessageHeader(MessageHeader const&) = default;
  MessageHeader() = default;
  MessageHeader(MessageHeader&&) = default;

  ::boost::optional<int> version;
  ::boost::optional<MessageType> type;
  ::boost::optional<unsigned> responseCode;
  ::boost::optional<std::string> database;
  ::boost::optional<RestVerb> restVerb;
  ::boost::optional<std::string> path;
  ::boost::optional<mapss> parameter;
  ::boost::optional<mapss> meta;
  ::boost::optional<std::string> user;
  ::boost::optional<std::string> password;
  ::boost::optional<ContentType> contentType;
};

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
         ,headerStrings(std::move(headerStrings))
         ,_sealed(false)
         ,_modified(true)
         ,_isVpack(boost::none)
         ,_builder(nullptr)
         {}

  Message(MessageHeader const& messageHeader
         ,mapss const& headerStrings
         )
         :header(messageHeader)
         ,headerStrings(headerStrings)
         ,_sealed(false)
         ,_modified(true)
         ,_isVpack(boost::none)
         ,_builder(nullptr)
         {}

  MessageHeader header;
  mapss headerStrings;
  uint64_t messageid; //generate by some singleton

  ///////////////////////////////////////////////
  // add payload
  ///////////////////////////////////////////////
  void addVPack(VSlice const& slice);
  void addVPack(VBuffer const& buffer);
  void addVPack(VBuffer&& buffer);
  void addBinary(uint8_t* data, std::size_t length);
  void addBinarySingle(VBuffer&& buffer);

  ///////////////////////////////////////////////
  // get payload
  ///////////////////////////////////////////////
  std::vector<VSlice>const & slices();
  std::pair<uint8_t const *, std::size_t> payload(); //as binary

  ContentType contentType(){ return header.contentType.get(); }

private:
  VBuffer _payload;
  bool _sealed;
  bool _modified;
  ::boost::optional<bool> _isVpack;
  std::shared_ptr<VBuilder> _builder;
  std::vector<VSlice> _slices;

};

class Request : public Message {
public:
  Request(MessageHeader&& messageHeader = MessageHeader()
         ,mapss&& headerStrings = mapss()
         ): Message(std::move(messageHeader), std::move(headerStrings))
         {}
  Request(MessageHeader const& messageHeader
         ,mapss const& headerStrings
         ): Message(messageHeader, headerStrings)
         {}

};

class Response : public Message {
public:
  Response(MessageHeader&& messageHeader = MessageHeader()
          ,mapss&& headerStrings = mapss()
          )
          :Message(std::move(messageHeader), std::move(headerStrings))
          {}


};

//std::unique_ptr<Response> createResponse(unsigned code);

}}}
