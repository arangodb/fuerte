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

namespace f = ::arangodb::fuerte;

class ConnectionBasicVstF : public ::testing::Test {
 protected:
  ConnectionBasicVstF(){
    _server = "vst://127.0.0.1:8530";
  }
  virtual ~ConnectionBasicVstF() noexcept {}

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
    _connection.reset();
  }

  std::shared_ptr<f::Connection> _connection;

 private:
  std::string _server;
  std::string _port;

};

namespace fu = ::arangodb::fuerte;

TEST_F(ConnectionBasicVstF, ApiVersionSync){
  auto request = fu::createRequest(fu::RestVerb::Get, "/_api/version");
  auto result = _connection->sendRequest(std::move(request));
  auto slice = result->slices().front();
  auto version = slice.get("version").copyString();
  auto server = slice.get("server").copyString();
  ASSERT_TRUE(server == std::string("arango")) << server << " == arango";
  ASSERT_TRUE(version[0] == '3');
}

TEST_F(ConnectionBasicVstF, ApiVersionASync){
  auto request = fu::createRequest(fu::RestVerb::Get, "/_api/version");
  fu::OnErrorCallback onError = [](fu::Error error, std::unique_ptr<fu::Request> req, std::unique_ptr<fu::Response> res){
    ASSERT_TRUE(false) << fu::to_string(fu::intToError(error));
  };
  fu::OnSuccessCallback onSuccess = [](std::unique_ptr<fu::Request> req, std::unique_ptr<fu::Response> res){
    auto slice = res->slices().front();
    auto version = slice.get("version").copyString();
    auto server = slice.get("server").copyString();
    ASSERT_TRUE(server == std::string("arango")) << server << " == arango";
    ASSERT_TRUE(version[0] == '3');
  };
  _connection->sendRequest(std::move(request),onError,onSuccess);
  fu::run();
}

TEST_F(ConnectionBasicVstF, ApiVersionSync20){
  auto request = fu::createRequest(fu::RestVerb::Get, "/_api/version");
  fu::Request req = *request;
  for(int i = 0; i < 20; i++){
    auto result = _connection->sendRequest(req);
    auto slice = result->slices().front();
    auto version = slice.get("version").copyString();
    auto server = slice.get("server").copyString();
    ASSERT_TRUE(server == std::string("arango")) << server << " == arango";
    ASSERT_TRUE(version[0] == '3');
  }
}

TEST_F(ConnectionBasicVstF, ApiVersionASync20){
  auto request = fu::createRequest(fu::RestVerb::Get, "/_api/version");
  fu::OnErrorCallback onError = [](fu::Error error, std::unique_ptr<fu::Request> req, std::unique_ptr<fu::Response> res){
    ASSERT_TRUE(false) << fu::to_string(fu::intToError(error));
  };
  fu::OnSuccessCallback onSuccess = [](std::unique_ptr<fu::Request> req, std::unique_ptr<fu::Response> res){
    auto slice = res->slices().front();
    auto version = slice.get("version").copyString();
    auto server = slice.get("server").copyString();
    ASSERT_TRUE(server == std::string("arango")) << server << " == arango";
    ASSERT_TRUE(version[0] == '3');
  };
  fu::Request req = *request;
  for(int i = 0; i < 20; i++){
    _connection->sendRequest(req,onError,onSuccess);
  }
  fu::run();
}

TEST_F(ConnectionBasicVstF, SimpleCursorSync){
  auto request = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
  fu::VBuilder builder;
  builder.openObject();
  builder.add("query", fu::VValue("FOR x IN 1..5 RETURN x"));
  builder.close();
  request->addVPack(builder.slice());
  auto result = _connection->sendRequest(std::move(request));
  auto slice = result->slices().front();


  std::cout << slice.toJson();
  //auto version = slice.get("version").copyString();
  //auto server = slice.get("server").copyString();
  //ASSERT_TRUE(server == std::string("arango")) << server << " == arango";
  //ASSERT_TRUE(version[0] == '3');
}

TEST_F(ConnectionBasicVstF, CreateDocumentSync){
  auto request = fu::createRequest(fu::RestVerb::Post, "/_api/document/_users");
  //fu::VBuilder builder;
  //builder.openObject();
  //builder.close();
  request->addVPack(fu::VSlice::emptyObjectSlice());
  auto result = _connection->sendRequest(std::move(request));
  auto slice = result->slices().front();


  std::cout << fu::to_string(slice) << std::endl;
  //auto version = slice.get("version").copyString();
  //auto server = slice.get("server").copyString();
  //ASSERT_TRUE(server == std::string("arango")) << server << " == arango";
  //ASSERT_TRUE(version[0] == '3');
}

TEST_F(ConnectionBasicVstF, ShortAndLongASync){
  fu::OnErrorCallback onError = [](fu::Error error, std::unique_ptr<fu::Request> req, std::unique_ptr<fu::Response> res){
    ASSERT_TRUE(false) << fu::to_string(fu::intToError(error));
  };

  fu::OnSuccessCallback onSuccess = [](std::unique_ptr<fu::Request> req, std::unique_ptr<fu::Response> res){
    auto slice = res->slices().front();
    std::cout << "messageID: " << req->messageid << " " << slice.toJson() << std::endl;
  };

  auto requestShort = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
  {
    fu::VBuilder builder;
    builder.openObject();
    builder.add("query", fu::VValue("RETURN SLEEP(1)"));
    builder.close();
    requestShort->addVPack(builder.slice());
  }

  auto requestLong = fu::createRequest(fu::RestVerb::Post, "/_api/cursor");
  {
    fu::VBuilder builder;
    builder.openObject();
    builder.add("query", fu::VValue("RETURN SLEEP(10)"));
    builder.close();
    requestLong->addVPack(builder.slice());
  }

  _connection->sendRequest(std::move(requestLong),onError,onSuccess);
  _connection->sendRequest(std::move(requestShort),onError,onSuccess);

  fu::run();
}
