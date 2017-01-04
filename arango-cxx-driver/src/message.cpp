#include <fuerte/message.h>
#include <fuerte/vst.h>

namespace arangodb { namespace fuerte { inline namespace v1 {

////helper
static bool specialHeader(Message& request, std::string const&, std::string const&){
  //static std::regex -- test
  if (/* maching condition*/ false ){
    //do special stuff on
    //request->bla
    return true;
  }
  return false;
}

//set value in the header
void setHeaderValue(Message request, std::string const& key, std::string const& val){
  if(!specialHeader(request, key, val)){
    request.headerStrings.emplace(key,val);
  }
};


//// external interface
Message createAuthMessage(std::string const& user, std::string const& password){
  Message request;
  request.messageHeader.type = MessageType::Authenticaton;
  request.messageHeader.user=user;
  request.messageHeader.password=password;
  return request;
}

Message createRequest(RestVerb verb
                     ,std::string const& database
                     ,std::string const& path
                     ,std::string const& user
                     ,std::string const& password
                     ,mapss parameter
                     ,mapss meta
                     ){

  //version must be set by protocol
  Message request;
  request.messageHeader.version = 0;
  request.messageHeader.type = MessageType::Request;
  request.messageHeader.responseCode = 0;
  request.messageHeader.database = database;
  request.messageHeader.requestType = verb;
  request.messageHeader.requestPath = path;
  request.messageHeader.requestPath = path;
  request.messageHeader.parameter = parameter;
  request.messageHeader.meta = meta;
  return request;
}

Message createResponse(unsigned code){
  Message request;
  //version must be set by protocol
  request.messageHeader.type = MessageType::Response;
  request.messageHeader.responseCode = code;
  return request;
}

///
NetBuffer toNetworkVst(Message const&){
  return "implement me";
}

NetBuffer toNetworkHttp(Message const&){
  return "implement me";
}

Message fromBufferVst(uint8_t const* begin, std::size_t length){
  auto num_slice = vst::validateAndCount(begin, begin + length);
  // work work
  return Message{};
}

boost::optional<Message>
fromNetworkVst(NetBuffer const& buffer
              ,vst::ReadBufferInfo& info
              ,vst::MessageMap& messageMap
              ){

  auto buff_begin = reinterpret_cast<uint8_t const*>(buffer.data());
  auto buff_end = buff_begin + buffer.size();
  auto chunkEndOffset = vst::isChunkComplete(buff_begin, buff_end, info);
  if (chunkEndOffset){ //chunk complete yeahy
    auto header = vst::readVstHeader(buff_begin, info);
    auto vpack_begin = buff_begin + header.headerLength;
    if(header.isFirst && header.chunk == 1) { // single chunk (message complete)
      return fromBufferVst(vpack_begin, chunkEndOffset);
    } else { //multichunk
      auto message_iter = messageMap.find(header.messageID);
      if(message_iter == messageMap.end()){
        //add new message
        std::pair<typename vst::MessageMap::iterator,bool> emplace_result
          = messageMap.emplace(header.messageID,vst::IncompleteMessage(header.messageLength,header.chunk));
        emplace_result.first->second.buffer.append(vpack_begin, header.chunkLength);
      } else {
        //continue old message
        vst::IncompleteMessage& m = message_iter->second;
        m.buffer.append(vpack_begin, header.chunkLength);
        message_iter->second.buffer.append(vpack_begin, header.chunkLength);
        if(m.numberOfChunks == header.chunk){
          return fromBufferVst( m.buffer.data() ,m.buffer.byteSize());
        }
      }
    }
  // todo store in info how much of the buffer we have processed
  // so the calling job can update its buffer
  }
  return boost::none;
}

boost::optional<Message> fromNetworkHttp(NetBuffer const& buffer){
  Message request;
  // parse body and convert to vpack
  return request;
}

}}}
