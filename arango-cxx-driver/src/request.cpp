#include <fuerte/request.h>
#include <fuerte/vst.h>

namespace arangodb { namespace rest { inline namespace v2 {

////helper
static bool specialHeader(Request& request, std::string const&, std::string const&){
  //static std::regex -- test
  if (/* maching condition*/ false ){
    //do special stuff on
    //request->bla
    return true;
  }
  return false;
}

//set value in the header
void setHeaderValue(Request request, std::string const& key, std::string const& val){
  if(!specialHeader(request, key, val)){
    request.headerStrings.emplace(key,val);
  }
};


//// external interface
Request createAuthRequest(std::string const& user, std::string const& password){
  Request request;
  request.reHeader.type = ReType::Authenticaton;
  request.reHeader.user=user;
  request.reHeader.password=password;
  return request;
}

Request createRequest(RestVerb verb
                     ,std::string const& database
                     ,std::string const& path
                     ,std::string const& user
                     ,std::string const& password
                     ,mapss parameter
                     ,mapss meta
                     ){

  //version must be set by protocol
  Request request;
  request.reHeader.version = 0;
  request.reHeader.type = ReType::Request;
  request.reHeader.responseCode = 0;
  request.reHeader.database = database;
  request.reHeader.requestType = verb;
  request.reHeader.requestPath = path;
  request.reHeader.requestPath = path;
  request.reHeader.parameter = parameter;
  request.reHeader.meta = meta;
  return request;
}

Request createResponse(unsigned code){
  Request request;
  //version must be set by protocol
  request.reHeader.type = ReType::Response;
  request.reHeader.responseCode = code;
  return request;
}

///
NetBuffer toNetworkVst(Request const&){
  return "implement me";
}

NetBuffer toNetworkHttp(Request const&){
  return "implement me";
}

Request fromBufferVst(uint8_t const* begin, std::size_t length){
  auto num_slice = vst::validateAndCount(begin, begin + length);
  // work work
  return Request{};
}

boost::optional<Request>
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

boost::optional<Request> fromNetworkHttp(NetBuffer const& buffer){
  Request request;
  // parse body and convert to vpack
  return request;
}

}}}
