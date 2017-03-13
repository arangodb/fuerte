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
#pragma once
#ifndef ARANGO_CXX_DRIVER_CONNECTION
#define ARANGO_CXX_DRIVER_CONNECTION

#include "types.h"
#include "connection_interface.h"

#include <memory>
#include <string>

namespace arangodb { namespace fuerte { inline namespace v1 {

class Database;

class Connection : public std::enable_shared_from_this<Connection> {
  friend class ConnectionBuilder;

  public:
    ~Connection(){ FUERTE_LOG_DEBUG << "DESTROYING CONNECTION" << std::endl; }
    void restart() { _realConnection->restart(); }
    std::shared_ptr<Database> getDatabase(std::string const& name);
    std::shared_ptr<Database> createDatabase(std::string const& name);
    bool deleteDatabase(std::string const& name);

    std::unique_ptr<Response> sendRequest(std::unique_ptr<Request> r){
      return _realConnection->sendRequest(std::move(r));
    }

    std::unique_ptr<Response> sendRequest(Request const& r){
      std::unique_ptr<Request> copy(new Request(r));
      return _realConnection->sendRequest(std::move(copy));
    }

    // callback may be called in parallel - think about possible races!
    MessageID sendRequest(std::unique_ptr<Request> r, OnErrorCallback e, OnSuccessCallback c){
      return _realConnection->sendRequest(std::move(r), e, c);
    }

    MessageID sendRequest(Request const& r, OnErrorCallback e, OnSuccessCallback c){
      std::unique_ptr<Request> copy(new Request(r));
      return _realConnection->sendRequest(std::move(copy), e, c);
    }

    std::size_t requestsLeft(){
      return _realConnection->requestsLeft();
    }

  private:
    Connection(detail::ConnectionConfiguration const& conf);
    std::shared_ptr<ConnectionInterface>  _realConnection;
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

}}}
#endif
