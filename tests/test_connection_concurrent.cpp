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

#include <fuerte/fuerte.h>
#include <fuerte/loop.h>
#include <fuerte/helper.h>

#include <thread>

#include "gtest/gtest.h"
#include "authentication_test.h"
#include "connection_test.h"

// Tesuite checks the thread-safety properties of the connection
// implementations. Try to send requests on the same connection object
// concurrently

// need a new class to use test fixture
class ConcurrentConnectionF : public ConnectionTestF {
  
  fu::StatusCode dropCollection() {
    auto request = fu::createRequest(fu::RestVerb::Delete, "/_api/collection/parallel");
    auto response = _connection->sendRequest(std::move(request));
    return response->statusCode();
  }
  
  virtual void SetUp() override {
    ConnectionTestF::SetUp();
    dropCollection(); // ignore response
    
    // create the collection
    fu::VBuilder builder;
    builder.openObject();
    builder.add("name", fu::VValue("parallel"));
    builder.close();
    auto request = fu::createRequest(fu::RestVerb::Post, "/_api/collection");
    request->addVPack(builder.slice());
    auto response = _connection->sendRequest(std::move(request));
    ASSERT_EQ(response->statusCode(), fu::StatusOK);
  }
  
  virtual void TearDown() override {
    // drop the collection
    ASSERT_EQ(dropCollection(), fu::StatusOK);
    ConnectionTestF::TearDown();
  }
    
};

TEST_P(ConnectionTestF, ApiVersionParallel) {
  fu::WaitGroup wg;
  std::atomic<size_t> counter(0);
  auto cb = [&](fu::Error error, std::unique_ptr<fu::Request> req, std::unique_ptr<fu::Response> res) {
    fu::WaitGroupDone done(wg);
    if (error) {
      ASSERT_TRUE(false) << fu::to_string(fu::intToError(error));
    } else {
      ASSERT_EQ(res->statusCode(), fu::StatusOK);
      auto slice = res->slices().front();
      auto version = slice.get("version").copyString();
      auto server = slice.get("server").copyString();
      ASSERT_EQ(server, "arango");
      ASSERT_EQ(version[0], _major_arango_version);
      counter.fetch_add(1, std::memory_order_relaxed);
    }
  };
  
  std::vector<std::thread> joins;
  for (size_t t = 0; t < threads(); t++) {
    wg.add(repeat());
    joins.emplace_back([&]{
      for (size_t i = 0; i < repeat(); i++) {
        auto request = fu::createRequest(fu::RestVerb::Get, "/_api/version");
        _connection->sendRequest(std::move(request), cb);
      }
    });
  }
  ASSERT_TRUE(wg.wait_for(std::chrono::seconds(60))); // wait for all threads to return

  // wait for all threads to end
  for (std::thread& t : joins) {
    t.join();
  }
  
  ASSERT_EQ(repeat() * threads(), counter);
}

TEST_P(ConcurrentConnectionF, CreateDocumentsParallel){
  
  std::atomic<size_t> counter(0);
  fu::WaitGroup wg;
  auto cb = [&](fu::Error error, std::unique_ptr<fu::Request> req,
                std::unique_ptr<fu::Response> res) {
    counter.fetch_add(1, std::memory_order_relaxed);
    fu::WaitGroupDone done(wg);
    if (error) {
      ASSERT_TRUE(false) << fu::to_string(fu::intToError(error));
    } else {
      ASSERT_EQ(res->statusCode(), fu::StatusAccepted);
      auto slice = res->slices().front();
      ASSERT_TRUE(slice.get("_id").isString());
      ASSERT_TRUE(slice.get("_key").isString());
      ASSERT_TRUE(slice.get("_rev").isString());
    }
  };
  
  fu::VBuilder builder;
  builder.openObject();
  builder.add("hello", fu::VValue("world"));
  builder.close();
  
  std::vector<std::thread> joins;
  for (size_t t = 0; t < threads(); t++) {
    wg.add(repeat());
    joins.emplace_back([&]{
      for (size_t i = 0; i < repeat(); i++) {
        auto request = fu::createRequest(fu::RestVerb::Post, "/_api/document/parallel");
        request->addVPack(builder.slice());
        _connection->sendRequest(std::move(request), cb);
      }
    });
  }
  ASSERT_TRUE(wg.wait_for(std::chrono::seconds(60))); // wait for all threads to return

  // wait for all threads to end
  for (std::thread& t : joins) {
    t.join();
  }
  
  ASSERT_EQ(repeat() * threads(), counter);
}


TEST_P(ConcurrentConnectionF, UpdateDocumentsParallel){
  // TODO
}

static const ConnectionTestParams params[] = {
  {._url= "http://127.0.0.1:8529", ._threads=2, ._repeat=1000},
  {._url= "vst://127.0.0.1:8529", ._threads=2, ._repeat=1000},
  {._url= "http://127.0.0.1:8529", ._threads=4, ._repeat=10000},
  {._url= "vst://127.0.0.1:8529", ._threads=4, ._repeat=10000},
};

INSTANTIATE_TEST_CASE_P(ConcurrentRequestsTests, ConcurrentConnectionF,
  ::testing::ValuesIn(params));

