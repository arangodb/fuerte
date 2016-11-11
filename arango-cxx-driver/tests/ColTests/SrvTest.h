#ifndef SRVTEST_H
#define SRVTEST_H

#include <gtest/gtest.h>
#include "fuerte/old/Server.h"

class SrvTest : public testing::Test {
 public:
  typedef arangodb::dbinterface::Server Server;
  typedef arangodb::dbinterface::Connection Connection;
  SrvTest();
  virtual ~SrvTest();

 protected:
  const Connection::VPack getDbVersion();

 private:
  Server::SPtr _pSrv;
  Connection::SPtr _pCon;
};

inline SrvTest::~SrvTest() {}

#endif  // SRVTEST_H
