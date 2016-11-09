#include <fuerte/next/request.h>
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
  request.user=user;
  request.user=password;
  return request;
}

Request createRequest(){
  Request request;
  request.reHeader.type = ReType::Request;
  return request;
}

Request createResponse(){
  Request request;
  request.reHeader.type = ReType::Response;
  return request;
}

///
NetBuffer toNetworkVst(Request const&){
  return "implement me";
}

NetBuffer toNetworkHttp(Request const&){
  return "implement me";
}

Request formNetworkVst(NetBuffer const&){
  Request request = createResponse();
  // contains slice?
  // parse slice
  return request;
}

Request formNetworkHttp(NetBuffer const&){
  Request request = createResponse();
  // parse body and convert to vpack
  return request;
}

}}}
