////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2017 ArangoDB GmbH, Cologne, Germany
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
/// @author Ewout Prangsma
////////////////////////////////////////////////////////////////////////////////

#pragma once 

#include <algorithm>
#include <stdlib.h>

#include <fuerte/fuerte.h>
#include <fuerte/loop.h>
#include <fuerte/helper.h>

#include "authentication_test.h"
#include "test_main.h"

namespace f = ::arangodb::fuerte;

typedef struct {
  const char *_url;       // Server URL
  const size_t _threads;  // #Threads to use for the EventLoopService 
  const size_t _repeat;   // Number of times to repeat repeatable tests.
} ConnectionTestParams;

::std::ostream& operator<<(::std::ostream& os, const ConnectionTestParams& p) {
  return os << "url=" << p._url << " threads=" << p._threads;
}

// ConnectionTestF is a test fixture that can be used for all kinds of connection 
// tests.
// You can configure it using the ConnectionTestParams struct.
class ConnectionTestF : public ::testing::TestWithParam<ConnectionTestParams> {
 public:
  const char _major_arango_version = '3';
 protected:
  ConnectionTestF() {
    _eventLoopService = std::unique_ptr<f::EventLoopService>(new f::EventLoopService(GetParam()._threads));
  }
  virtual ~ConnectionTestF() noexcept {}

  virtual void SetUp() override {
    try {
      // Set connection parameters
      f::ConnectionBuilder cbuilder;
      cbuilder.host(GetParam()._url);
      setupAuthenticationFromEnv(cbuilder);

      // make connection
      _connection = cbuilder.connect(*_eventLoopService);
    } catch(std::exception const& ex) {
      std::cout << "SETUP OF FIXTURE FAILED" << std::endl;
      throw ex;
    }
  }

  virtual void TearDown() override {
    _connection.reset();
  }

  // Number of times to repeat certain tests.
  inline size_t repeat() {
    return std::max(GetParam()._repeat, size_t(1));
  }

  std::shared_ptr<f::Connection> _connection;

 private:
  std::unique_ptr<f::EventLoopService> _eventLoopService;
};

