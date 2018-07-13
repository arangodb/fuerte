////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2018 ArangoDB GmbH, Cologne, Germany
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
/// @author Simon Grätzer
////////////////////////////////////////////////////////////////////////////////
#include "test_main.h"
#include "authentication_test.h"

#include <fuerte/fuerte.h>
#include <fuerte/requests.h>
#include <velocypack/velocypack-aliases.h>

namespace fu = ::arangodb::fuerte;


TEST(RequestTimeout, HTTP) {
  fu::EventLoopService loop;
  // Set connection parameters
  fu::ConnectionBuilder cbuilder;
  cbuilder.host("http://127.0.0.1:8529");
  setupAuthenticationFromEnv(cbuilder);
  
  // make connection
  auto connection = cbuilder.connect(loop);
  
  // should fail after 1s
  auto req = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
  req->timeout(std::chrono::milliseconds(1000));
  {
    VPackBuilder builder;
    builder.openObject();
    builder.add("query", VPackValue("RETURN SLEEP(2)"));
    builder.close();
    req->addVPack(builder.slice());
  }
  
  fu::WaitGroup wg;
  wg.add();
  connection->sendRequest(std::move(req), [&](fu::Error e, std::unique_ptr<fu::Request> req,
                                              std::unique_ptr<fu::Response>) {
    fu::WaitGroupDone done(wg);
    ASSERT_EQ(fu::intToError(e), fu::ErrorCondition::Timeout);
  });
  ASSERT_TRUE(wg.wait_for(std::chrono::milliseconds(10000)));
  
  // connection must now reconnect and work again
  req = fu::createRequest(fu::RestVerb::Post, "/_api/version");
  wg.add();
  connection->sendRequest(std::move(req), [&](fu::Error e, std::unique_ptr<fu::Request> req,
                                              std::unique_ptr<fu::Response> res) {
    fu::WaitGroupDone done(wg);
    if (e) {
      ASSERT_TRUE(false) << fu::to_string(fu::intToError(e));
    } else {
      ASSERT_EQ(res->statusCode(), fu::StatusOK);
      auto slice = res->slices().front();
      auto version = slice.get("version").copyString();
      auto server = slice.get("server").copyString();
      ASSERT_EQ(server, "arango");
      ASSERT_EQ(version[0], '3'); // major version
    }
  });
  wg.wait();
}

TEST(Timeouts, VelocyStream){
  ASSERT_TRUE(true); //TODO -- DELETE
  
  
}
