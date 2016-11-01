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


/** The connection Builder is a class that allows the easy configuration of
 *  connections. We decided to use the builder pattern because the connections
 *  have too many options to put them all in a single constructor. When you have
 *  passed all your options call connect() in order to receive a shared_ptr to a
 *  connection. Remember to use the "->" operator to access the connections
 *  members.
 */
class ConnectionBuilder {
  public:
    ConnectionBuilder& host(std::string const&); // takes url in the form  (http|vst)[s]://(ip|hostname):port
                                                 // sets protocol host and port
    //ConnectionBuilder() = delete;
    //ConnectionBuilder(std::string const& s){
    //  host(s);
    //};

    std::shared_ptr<Connection> connect(){
      return std::shared_ptr<Connection>( new Connection(_conf)) ;
    }

    ConnectionBuilder& async(bool b){ _conf._async = b; return *this; }
    ConnectionBuilder& user(std::string const& u){ _conf._user = u; return *this; }
    ConnectionBuilder& password(std::string const& p){ _conf._password = p; return *this; }
    ConnectionBuilder& maxChunkSize(std::size_t c){ _conf._maxChunkSize = c; return *this; }

  private:
    detail::ConnectionConfiguration _conf;
};

}
#endif
