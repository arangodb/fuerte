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

#include <fuerte/message.h>
#include <velocypack/Validator.h>
#include <sstream>

#include "vst.h"

namespace arangodb { namespace fuerte { inline namespace v1 {

std::string to_string(MessageHeader const& header){
  ::boost::optional<int> version;
  ::boost::optional<MessageType> type;
  ::boost::optional<unsigned> responseCode;
  ::boost::optional<std::string> database;
  ::boost::optional<RestVerb> restVerb;           // GET POST ...
  ::boost::optional<std::string> path;            // equivalent of http path
  ::boost::optional<mapss> parameter;             // equivalent of http parametes ?foo=bar
  ::boost::optional<mapss> meta;                  // equivalent of http headers
  ::boost::optional<std::string> user;
  ::boost::optional<std::string> password;
  std::stringstream ss;

  if(header.byteSize){
    ss << "byteSize: " << header.byteSize.get() << std::endl;
  }

  if(header.version){
    ss << "version: " << header.version.get() << std::endl;
  }

  if(header.type){
    ss << "type: " << static_cast<int>(header.type.get()) << std::endl;
  }

  if(header.responseCode){
    ss << "responseCode: " << header.responseCode.get() << std::endl;
  }

  if(header.database){
    ss << "database: " << header.database.get() << std::endl;
  }

  if(header.restVerb){
    ss << "restVerb: " << to_string(header.restVerb.get()) << std::endl;
  }

  if(header.path){
    ss << "path: " << header.path.get() << std::endl;
  }

  if(header.parameter){
    ss << "parameters: ";
    for(auto const& item : header.parameter.get()){
      ss << item.first <<  " -:- " << item.second << "\n";
    }
    ss<< std::endl;
  }

  if(header.meta){
    ss << "meta:\n";
    for(auto const& item : header.meta.get()){
      ss << "\t" << item.first <<  " -:- " << item.second << "\n";
    }
    ss<< std::endl;
  }

  if(header.user){
    ss << "user: " << header.user.get() << std::endl;
  }

  if(header.password){
    ss << "password: " << header.password.get() << std::endl;
  }

  ss << "contentType: " << header.contentTypeString() << std::endl;

  return ss.str();
}

///////////////////////////////////////////////
// class MessageHeader
///////////////////////////////////////////////

// content type accessors
std::string MessageHeader::contentTypeString() const {
  if(!meta){
    return "";
  }
  auto& hmap = meta.get();
  auto found =  hmap.find(fu_content_type_key);
  if(found == hmap.end()){
    return "";
  }
  return found->second;
}

ContentType MessageHeader::contentType() const {
  return to_ContentType(contentTypeString());
}

void MessageHeader::contentType(std::string const& type){
  if (!meta) {
    meta = mapss();
  }
  meta.get()[fu_content_type_key] = type;
}

void MessageHeader::contentType(ContentType type){
  contentType(to_string(type));
}

// accept header accessors
std::string MessageHeader::acceptTypeString() const {
  if(!meta){
    return "";
  }
  auto& hmap = meta.get();
  auto found =  hmap.find(fu_accept_key);
  if(found == hmap.end()){
    return "";
  }
  return found->second;
}

ContentType MessageHeader::acceptType() const {
  return to_ContentType(acceptTypeString());
}

void MessageHeader::acceptType(std::string const& type){
  if(!meta){
    meta = mapss();
  }
  FUERTE_LOG_DEBUG << "setting Accept to: " << type << std::endl;
  meta.get()[fu_accept_key] = type;
}

void MessageHeader::acceptType(ContentType type){
  acceptType(to_string(type));
}

///////////////////////////////////////////////
// class Message
///////////////////////////////////////////////

// content-type header accessors
std::string Message::contentTypeString() const {
  return header.contentTypeString();
}

ContentType Message::contentType() const {
  return header.contentType();
}

void Request::contentType(std::string const& type) {
  header.contentType(type);
}

void Request::contentType(ContentType type) {
  header.contentType(type);
}

// accept header accessors
std::string Message::acceptTypeString() const {
  return header.acceptTypeString();
}

ContentType Message::acceptType() const {
  return header.acceptType();
}

///////////////////////////////////////////////
// class Request
///////////////////////////////////////////////

void Request::acceptType(std::string const& type) {
  header.acceptType(type);
}

void Request::acceptType(ContentType type) {
  header.acceptType(type);
}

//// add payload
// add VelocyPackData
void Request::addVPack(VSlice const& slice){
#ifdef FUERTE_CHECKED_MODE
  //FUERTE_LOG_ERROR << "Checking data that is added to the message: " << std::endl;
  vst::validateAndCount(slice.start(),slice.byteSize());
#endif
  if(_sealed || (_isVpack && !_isVpack.get())){
    throw std::logic_error("Message is sealed or of wrong type (vst/binary)");
  };

  if(!_builder){
    _builder = std::make_shared<VBuilder>(_payload);
  }

  contentType(ContentType::VPack);
  _isVpack=true;
  _modified = true;
  _builder->add(slice);
  _payloadLength += slice.byteSize();
  _payload.resetTo(_payloadLength);
}

void Request::addVPack(VBuffer const& buffer){
#ifdef FUERTE_CHECKED_MODE
  //FUERTE_LOG_ERROR << "Checking data that is added to the message: " << std::endl;
  vst::validateAndCount(buffer.data(),buffer.byteSize());
#endif
  if(_sealed || (_isVpack && !_isVpack.get())){
    throw std::logic_error("Message is sealed or of wrong type (vst/binary)");
  };
  _isVpack = true;
  contentType(ContentType::VPack);
  _modified = true;
  _modified = true;
  auto length = buffer.byteSize();
  auto cursor = buffer.data();

  if(!_builder){
    _builder = std::make_shared<VBuilder>(_payload);
  }

  while(length){
    VSlice slice(cursor);
    _builder->add(slice);
    auto sliceSize = _slices.back().byteSize();
    if(length < sliceSize){
      throw std::logic_error("invalid buffer");
    }
    cursor += sliceSize;
    length -= sliceSize;
    _payloadLength += sliceSize;
    _payload.resetTo(_payloadLength);
  }
}

void Request::addVPack(VBuffer&& buffer){
#ifdef FUERTE_CHECKED_MODE
  //FUERTE_LOG_ERROR << "Checking data that is added to the message: " << std::endl;
  vst::validateAndCount(buffer.data(),buffer.byteSize());
#endif
  if(_sealed || _isVpack){
    throw std::logic_error("Message is sealed or of wrong type (vst/binary)");
  };
  contentType(ContentType::VPack);
  _isVpack = true;
  _sealed = true;
  _modified = true;
  _payloadLength += buffer.byteSize();
  _payload = std::move(buffer);
  _payload.resetTo(_payloadLength);
}

// add binary data
void Request::addBinary(uint8_t const* data, std::size_t length){
  if(_sealed || (_isVpack && _isVpack.get())){ return; };
  _isVpack = false;
  _modified = true;
  _payloadLength += length;
  _payload.append(data, length); //TODO reset to!!! FIXME
  _payload.resetTo(_payloadLength);
}

void Request::addBinarySingle(VBuffer&& buffer){
  if(_sealed || (_isVpack && _isVpack.get())){ return; };
  _isVpack = false;
  _sealed = true;
  _modified = true;
  _payloadLength += buffer.byteSize();
  _payload = std::move(buffer);
  _payload.resetTo(_payloadLength);
}

// get payload as slices
std::vector<VSlice>const & Request::slices() {
  if(_isVpack && _modified) {
    _slices.clear();
    auto length = _payload.byteSize();
    auto cursor = _payload.data();
    while(length){
      _slices.emplace_back(cursor);
      auto sliceSize = _slices.back().byteSize();
      if(length < sliceSize){
        throw std::logic_error("invalid buffer");
      }
      cursor += sliceSize;
      length -= sliceSize;
    }
    _modified = false;
  }
  return _slices;
}

// get payload as binary
boost::asio::const_buffer Request::payload() const {
  return boost::asio::const_buffer(_payload.data(), _payloadLength);
}

///////////////////////////////////////////////
// class Response
///////////////////////////////////////////////

std::vector<VSlice>const & Response::slices() {
  if (_slices.empty()) {
    auto length = _payload.byteSize() - _payloadOffset;
    auto cursor = _payload.data() + _payloadOffset;
    while (length){
      _slices.emplace_back(cursor);
      auto sliceSize = _slices.back().byteSize();
      if (length < sliceSize){
        throw std::logic_error("invalid buffer");
      }
      cursor += sliceSize;
      length -= sliceSize;
    }
  }
  return _slices;
}

boost::asio::const_buffer Response::payload() const {
  return boost::asio::const_buffer(_payload.data() + _payloadOffset, _payload.byteSize());
}

void Response::setPayload(VBuffer&& buffer, size_t payloadOffset) {
  _slices.clear();
  _payloadOffset = payloadOffset;
  _payload = std::move(buffer);
}


// ////helper
// static bool specialHeader(Message& request, std::string const&, std::string const&){
//   //static std::regex -- test
//   if (/* maching condition*/ false ){
//     //do special stuff on
//     //request->bla
//     return true;
//   }
//   return false;
// }
//
// //set value in the header
// void setHeaderValue(Message request, std::string const& key, std::string const& val){
//   if(!specialHeader(request, key, val)){
//     request.headerStrings.emplace(key,val);
//   }
// };
//
// //// external interface
// Message createAuthMessage(std::string const& user, std::string const& password){
//   Message request;
//   request.header.type = MessageType::Authentication;
//   request.header.user=user;
//   request.header.password=password;
//   return request;
// }
//
// Message createRequest(RestVerb verb
//                      ,std::string const& database
//                      ,std::string const& path
//                      ,std::string const& user
//                      ,std::string const& password
//                      ,mapss parameter
//                      ,mapss meta
//                      ){
//
//   //version must be set by protocol
//   Message request;
//   request.header.version = 0;
//   request.header.type = MessageType::Request;
//   request.header.responseCode = 0;
//   request.header.database = database;
//   request.header.restVerb = verb;
//   request.header.path = path;
//   request.header.parameter = parameter;
//   request.header.meta = meta;
//   return request;
// }
//
// std::unique_ptr<Response> createResponse(unsigned code){
//   auto response = std::unique_ptr<Response>(new Response());
//   //version must be set by protocol
//   response->header.type = MessageType::Response;
//   response->header.responseCode = code;
//   return response;
// }

}}}
