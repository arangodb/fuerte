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
  request.header.type = MessageType::Authentication;
  request.header.user=user;
  request.header.password=password;
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
  request.header.version = 0;
  request.header.type = MessageType::Request;
  request.header.responseCode = 0;
  request.header.database = database;
  request.header.restVerb = verb;
  request.header.path = path;
  request.header.parameter = parameter;
  request.header.meta = meta;
  return request;
}

std::unique_ptr<Response> createResponse(unsigned code){
  auto response = std::unique_ptr<Response>(new Response());
  //version must be set by protocol
  response->header.type = MessageType::Response;
  response->header.responseCode = code;
  return response;
}

}}}
