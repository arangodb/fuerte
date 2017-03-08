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

#include "test_main.h"
#include <fuerte/fuerte.h>
#include <fuerte/loop.h>
#include <fuerte/helper.h>

#include <velocypack/Parser.h>
#include <velocypack/Iterator.h>
#include <velocypack/velocypack-aliases.h>

#include <iostream>
#include <fstream>

namespace f = ::arangodb::fuerte;

class Connection100kWritesF : public ::testing::Test {
 protected:
  Connection100kWritesF(){
    _server = "vst://127.0.0.1:8530";
  }
  virtual ~Connection100kWritesF() noexcept {}

  virtual void SetUp() override {
    try {
      f::ConnectionBuilder cbuilder;
      cbuilder.host(_server);
      _connection = cbuilder.connect();
    } catch(std::exception const& ex) {
      std::cout << "SETUP OF FIXTURE FAILED" << std::endl;
      throw ex;
    }
  }

  virtual void TearDown() override {
    //VPackBuilder builder;
    //builder.openObject();
    //builder.add("name" , VPackValue("_testobi"));
    //builder.close();
    auto request = arangodb::fuerte::createRequest(
        arangodb::fuerte::RestVerb::Delete, "/_api/collection/_testobi");
    //request->addVPack(builder.slice());
    auto result = _connection->sendRequest(std::move(request));

    arangodb::fuerte::run();
    _connection.reset();
  }

  std::shared_ptr<f::Connection> _connection;

 private:
  std::string _server;
  std::string _port;

};

namespace fu = ::arangodb::fuerte;

TEST_F(Connection100kWritesF, Writes100k){

  std::ifstream fh("bodies-full.json");
  ASSERT_TRUE(fh.is_open());
  std::stringstream ss;
  ss << fh.rdbuf();
  auto content = ss.str();

  VPackParser parser;
  parser.parse(content);
  auto builder = parser.steal();

  fu::OnErrorCallback onError = [](fu::Error error, std::unique_ptr<fu::Request> req, std::unique_ptr<fu::Response> res){
    ASSERT_TRUE(false) << fu::to_string(fu::intToError(error));
    throw;
  };

  fu::OnSuccessCallback onSuccess = [](std::unique_ptr<fu::Request> req, std::unique_ptr<fu::Response> res){
    //ASSERT_TRUE(res->header.responseCode.get() < 400);
    std::cerr << res->messageid << std::endl;
    if (res->header.responseCode.get() >= 400){
      std::cerr << fu::to_string(*req);
      std::cerr << fu::to_string(*res);
      ASSERT_TRUE(false);
      throw;
    }
  };

  {
    auto request = arangodb::fuerte::createRequest(
        arangodb::fuerte::RestVerb::Delete, "/_api/collection/testobi");
    auto result = _connection->sendRequest(std::move(request));
    arangodb::fuerte::run();
  }

  {
    VPackBuilder builder;
    builder.openObject();
    builder.add("name" , VPackValue("testobi"));
    builder.close();
    arangodb::fuerte::Request request = *fu::createRequest(fu::RestVerb::Post, "/_api/collection");
    request.addVPack(builder.slice());
    auto result = _connection->sendRequest(std::move(request));
    if (result->header.responseCode.get() >= 400){
      std::cerr << fu::to_string(request);
      std::cerr << fu::to_string(*result);
      ASSERT_TRUE(false);
      throw;
    }
  }

  std::cerr << "enter loop" << std::endl;
  std::size_t i = 0;
  for(auto const& slice : VPackArrayIterator(builder->slice())){
    ++i;
    if (i % 50 == 0){
      fu::run();
    }
    auto request = fu::createRequest(fu::RestVerb::Post, "/_api/document/testobi");
    //request->addVPack(slice);
    request->addBinary(slice.start(),slice.byteSize());
    _connection->sendRequest(std::move(request), onError, onSuccess);
  }
  fu::run();
}

