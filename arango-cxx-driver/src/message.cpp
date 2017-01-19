#include <fuerte/message.h>
#include <fuerte/vst.h>
#include <velocypack/Validator.h>

namespace arangodb { namespace fuerte { inline namespace v1 {

///////////////////////////////////////////////
// class Message
///////////////////////////////////////////////

//// add payload
// add VelocyPackData
void Message::addVPack(VSlice const& slice){
  if(_sealed || (_isVpack && !_isVpack.get())){
    throw std::logic_error("Message is sealed or of wrong type (vst/binary)");
  };

  if(!_builder){
    _builder = std::make_shared<VBuilder>(_payload);
  }

  _isVpack=true;
  _modified = true;
  _builder->add(slice);
}

void Message::addVPack(VBuffer const& buffer){
  if(_sealed || (_isVpack && !_isVpack.get())){
    throw std::logic_error("Message is sealed or of wrong type (vst/binary)");
  };
  _isVpack = true;
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
  }
}

void Message::addVPack(VBuffer&& buffer){
  if(_sealed || _isVpack){
    throw std::logic_error("Message is sealed or of wrong type (vst/binary)");
  };
  _isVpack = true;
  _sealed = true;
  _modified = true;
  _payload = std::move(buffer);
}

// add binary data
void Message::addBinary(uint8_t* data, std::size_t length){
  if(_sealed || (_isVpack && _isVpack.get())){ return; };
  _isVpack = false;
  _modified = true;
  if(!_builder){
    _builder = std::make_shared<VBuilder>(_payload);
  }
}

void Message::addBinarySingle(VBuffer&& buffer){
  if(_sealed || (_isVpack && _isVpack.get())){ return; };
  _isVpack = false;
  _sealed = true;
  _modified = true;
  _payload = std::move(buffer);
}


//// get payload
// get payload as slices
std::vector<VSlice>const & Message::slices(){
  if(_isVpack && _modified){
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
std::pair<uint8_t const *, std::size_t> Message::payload(){
  return { _payload.data(), _payload.byteSize() };
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
