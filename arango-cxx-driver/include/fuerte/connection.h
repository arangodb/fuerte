#pragma once
#ifndef ARANGO_CXX_DRIVER_CONNECTION
#define ARANGO_CXX_DRIVER_CONNECTION

#include <memory>
#include <string>
#include "common_types.h"

namespace arangocxx {

class Database;

class Connection : public std::enable_shared_from_this<Connection> {
  friend class ConnectionBuilder;

    Connection(detail::ConnectionConfiguration conf):
    _configuration(conf)
    {};

  public:
    std::shared_ptr<Database> getDatabase(std::string name);
    std::shared_ptr<Database> createDatabase(std::string name);
    bool deleteDatabase(std::string name);

  private:
    detail::ConnectionConfiguration _configuration;

};

class ConnectionBuilder {
  public:
    std::shared_ptr<Connection> connect(){
      return std::shared_ptr<Connection>( new Connection(_conf)) ;
    }
    void host(std::string const&); // takes url in the form  (http|vst)[s]://(ip|hostname):port
                                   // sets protocol host and port
    void async(bool b){ _conf._async = b; }
    void user(std::string const& u){ _conf._user = u; }
    void password(std::string const& p){ _conf._password = p; }
    void maxChunkSize(std::size_t c){ _conf._maxChunkSize = c; }

  private:
    detail::ConnectionConfiguration _conf;
};

}
#endif
