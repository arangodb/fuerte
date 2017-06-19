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

#include <memory>

#include <boost/asio/io_service.hpp>
#include <curl/curl.h>

#include <fuerte/FuerteLogger.h>
#include <fuerte/loop.h>
#include <fuerte/types.h>

#include "VpackInit.h"

namespace arangodb { namespace fuerte { inline namespace v1 {

class VstConnection;

GlobalService::GlobalService() :
  _vpack_init(new impl::VpackInit()) { 
  FUERTE_LOG_DEBUG << "GlobalService init" << std::endl;
  ::curl_global_init(CURL_GLOBAL_ALL); 
}
GlobalService::~GlobalService() { 
  FUERTE_LOG_DEBUG << "GlobalService cleanup" << std::endl;
  ::curl_global_cleanup(); 
}

EventLoopService::EventLoopService(unsigned int threadCount)
    : EventLoopService(threadCount, std::make_shared<asio_io_service>()) {}

// run is called for each thread. It calls io_service.run() and
// invokes the curl handlers.
// You only need to invoke this if you want a custom event loop service.
void EventLoopService::run() {
  try {
    io_service_->run();
  } catch (std::exception const& ex) {
    handleRunException(ex);
  }
}

}}}
