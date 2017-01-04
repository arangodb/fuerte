#include <fuerte/connection.h>
#include <fuerte/database.h>
#include "HttpConnection.h"

#include <boost/algorithm/string.hpp>
#include <vector>

namespace arangodb { namespace fuerte { inline namespace v1 {
  using namespace arangodb::fuerte::detail;

  Connection::Connection(detail::ConnectionConfiguration conf):
    _realConnection(nullptr),
    _configuration(conf)
    {
      if (_configuration._connType == TransportType::Vst){
        //_realConnection = std::make_shared<VstConnection>(ioservice, _configuration)
      } else {
        auto communicator = std::make_shared<http::HttpCommunicator>();
        _realConnection = std::make_shared<http::HttpConnection>(communicator, _configuration);
      }
    };

  ConnectionBuilder& ConnectionBuilder::host(std::string const& str){
    std::vector<std::string> strings;
    boost::split(strings, str, boost::is_any_of(":"));

    //get protocol
    std::string const& proto = strings[0];
    if (proto == "vst"){
      _conf._connType = TransportType::Vst;
      _conf._ssl = false;
    }
    else if (proto == "vsts"){
      _conf._connType = TransportType::Vst;
      _conf._ssl = true;
    }
    else if (proto == "http"){
      _conf._connType = TransportType::Http;
      _conf._ssl = false;
    }
    else if (proto == "https"){
      _conf._connType = TransportType::Http;
      _conf._ssl = true;
    }
    else {
      throw "invalid protocol";
    }

    //do more checking?
    _conf._host = strings[1].erase(0,2); //remove '//'
    _conf._port = strings[2];

    return *this;
  }

  // TODO add back later as soon basic functionality has been provided
  // // get or create?!
  // std::shared_ptr<Database> Connection::getDatabase(std::string name){
  //   return std::shared_ptr<Database> ( new Database(shared_from_this(), name));
  // }
  // std::shared_ptr<Database> Connection::createDatabase(std::string name){
  //   return std::shared_ptr<Database> ( new Database(shared_from_this(), name));
  // }
  // bool Connection::deleteDatabase(std::string name){
  //   return false;
  // }

}}}
