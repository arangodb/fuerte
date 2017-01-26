#include "test_main.h"
#include <fuerte/arangocxx.h>
#include <fuerte/loop.h>

namespace f = ::arangodb::fuerte;

class BasicsF : public ::testing::Test {
 protected:
  BasicsF(){
    _server = "vst://127.0.0.1:8529";
  }
  virtual ~BasicsF() noexcept {}

  virtual void SetUp() override {
    f::ConnectionBuilder cbuilder;
    cbuilder.host(_server);
    _connection = cbuilder.connect();
    f::getProvider().pollAsio(true);
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

TEST_F(BasicsF, version){
  ASSERT_TRUE(true);
  auto request = fu::createRequest(fu::RestVerb::Get, "/api/version");
  auto result = _connection->sendRequest(std::move(request));
  std::cout << fu::to_string(result->header);
  std::cout << fu::to_string(result->slices()[0]);
}
