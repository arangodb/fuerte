#include "test_main.h"
#include <fuerte/arangocxx.h>
#include <fuerte/loop.h>

namespace f = ::arangodb::fuerte;

class BasicsF : public ::testing::Test {
 protected:
  BasicsF(){
    _server = "vst://127.0.0.1:8529";

    f::ConnectionBuilder cbuilder;
    cbuilder.host(_server);
    _connection = cbuilder.connect();


  }
  virtual ~BasicsF() noexcept {}
  virtual void SetUp() override {}
  virtual void TearDown() override {}

 private:
  std::string _server;
  std::string _port;
  std::shared_ptr<f::Connection> _connection;

};

TEST_F(BasicsF, version){
  ASSERT_TRUE(true);
  std::cout << "in the test method" << std::endl;
  f::poll(true);
}
