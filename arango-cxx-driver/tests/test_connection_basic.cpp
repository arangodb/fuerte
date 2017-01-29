#include "test_main.h"
#include <fuerte/arangocxx.h>
#include <fuerte/loop.h>

namespace f = ::arangodb::fuerte;

class ConnectionBasicF : public ::testing::Test {
 protected:
  ConnectionBasicF(){
    _server = "vst://127.0.0.1:8529";
  }
  virtual ~ConnectionBasicF() noexcept {}

  virtual void SetUp() override {
    f::ConnectionBuilder cbuilder;
    cbuilder.host(_server);
    _connection = cbuilder.connect();
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

TEST_F(ConnectionBasicF, ApiVersionSync){
  ASSERT_TRUE(true);
  auto request = fu::createRequest(fu::RestVerb::Get, "/_api/version");
  auto result = _connection->sendRequest(std::move(request));
  auto slice = result->slices().front();
  auto version = slice.get("version").copyString();
  auto server = slice.get("server").copyString();
  ASSERT_TRUE(server == std::string("arango")) << server << " == arango";
  ASSERT_TRUE(version[0] == '3');
}

TEST_F(ConnectionBasicF, ApiVersionASync){
  ASSERT_TRUE(true);
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

TEST_F(ConnectionBasicF, ApiVersionSync20){
  ASSERT_TRUE(true);
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

TEST_F(ConnectionBasicF, ApiVersionASync20){
  ASSERT_TRUE(true);
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

