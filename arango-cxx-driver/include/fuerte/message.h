#pragma once

#include "common_types.h"

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

    // add VelocyPackData
    void addVPack(VSlice const& slice){
      if(_sealed || (_isVpack && !_isVpack.get())){
        throw std::logic_error("Message is sealed or of wrong type (vst/binary)");
      };

      if(!_builder){
        _builder = std::make_shared<VBuilder>(_payload);
      }

      _isVpack=true;
      _modified = true;
      _builder->add(slice);
      // add slice to _slices

    }
    void addVPack(VBuffer const& buffer){
      if(_sealed || (_isVpack && !_isVpack.get())){
        throw std::logic_error("Message is sealed or of wrong type (vst/binary)");
      };
      auto length = buffer.byteSize();
      auto cursor = buffer.data();

      if(!_builder){
        _builder = std::make_shared<VBuilder>(_payload);
      }

      while(length){
        VSlice slice(cursor);
        _builder->add(slice);
        addVPack(slice);
        cursor += slice.byteSize();
        length -= slice.byteSize();
      }
    }

    void addVPack(VBuffer&& buffer){
      if(_sealed || _isVpack){
        throw std::logic_error("Message is sealed or of wrong type (vst/binary)");
      };
      _isVpack = true;
      _sealed = true;
      _modified = true;
      _payload = std::move(buffer);
    }

    // add binary data

    void addBinary(uint8_t* data, std::size_t length){
      if(_sealed || (_isVpack && _isVpack.get())){ return; };
      _isVpack = false;
      _modified = true;
      if(!_builder){
        _builder = std::make_shared<VBuilder>(_payload);
      }
    }

    void addBinarySingle(VBuffer&& buffer){
      if(_sealed || (_isVpack && _isVpack.get())){ return; };
      _isVpack = false;
      _sealed = true;
      _modified = true;
      _payload = std::move(buffer);
    }


    ///////////////////////////////////////////////
    // get payload
    ///////////////////////////////////////////////

    // get payload as slices
    std::vector<VSlice>const & slices(){
      if(_isVpack && _modified){
        _slices.clear();
        auto length = _payload.byteSize();
        auto cursor = _payload.data();
        while(length){
          _slices.emplace_back(cursor);
          cursor += _slices.back().byteSize();
          length -= _slices.back().byteSize();
        }
      }
      return _slices;
    }

    //get payload as binary
    std::pair<uint8_t const *, std::size_t> payload(){
      return { _payload.data(), _payload.byteSize() };
    }

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

std::unique_ptr<Response> createResponse(unsigned code);

}}}
