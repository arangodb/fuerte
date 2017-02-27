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

#include <fuerte/connection.h>
#include <fuerte/database.h>
#include "HttpConnection.h"
#include "VstConnection.h"

#include <boost/algorithm/string.hpp>
#include <vector>

namespace arangodb { namespace fuerte { inline namespace v1 {
  using namespace arangodb::fuerte::detail;

  Connection::Connection(detail::ConnectionConfiguration const& conf):
    _realConnection(nullptr),
    _configuration(conf)
    {
      if (_configuration._connType == TransportType::Vst){
        FUERTE_LOG_DEBUG << "creating velocystream connection" << std::endl;
        _realConnection = std::make_shared<vst::VstConnection>(_configuration);
        _realConnection->start();
      } else {
        FUERTE_LOG_DEBUG << "creating http connection" << std::endl;
        _realConnection = std::make_shared<http::HttpConnection>(_configuration);
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
      throw std::runtime_error(std::string("invalid protocol: ") + proto);
    }

    //TODO
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
