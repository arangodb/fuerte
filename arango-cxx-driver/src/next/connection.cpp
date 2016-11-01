#include <fuerte/next/connection.h>
#include <fuerte/next/database.h>
#include <boost/algorithm/string.hpp>

namespace arangocxx {
  using namespace arangocxx::detail;

  ConnectionBuilder& ConnectionBuilder::host(std::string const& str){
    std::vector<std::string> strings;
    boost::split(strings, str, boost::is_any_of(":"));

    //get protocol
    std::string const& proto = strings[0];
    if (proto == "vst"){
      _conf._connType = ConnectionType::vst;
      _conf._ssl = false;
    }
    else if (proto == "vsts"){
      _conf._connType = ConnectionType::vst;
      _conf._ssl = true;
    }
    else if (proto == "http"){
      _conf._connType = ConnectionType::http;
      _conf._ssl = false;
    }
    else if (proto == "https"){
      _conf._connType = ConnectionType::http;
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

  // get or create?!
  std::shared_ptr<Database> Connection::getDatabase(std::string name){
    return std::shared_ptr<Database> ( new Database(shared_from_this(), name));
  }
  std::shared_ptr<Database> Connection::createDatabase(std::string name){
    return std::shared_ptr<Database> ( new Database(shared_from_this(), name));
  }
  bool Connection::deleteDatabase(std::string name){
    return false;
  }

}
