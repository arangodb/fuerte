#ifndef DBTEST_H
#define DBTEST_H

#include <gtest/gtest.h>

#include "arangodbcpp/Server.h"
#include "arangodbcpp/Database.h"

class DbTest : public testing::Test {
 public:
  typedef arangodb::dbinterface::Connection Connection;
  typedef arangodb::dbinterface::Database Database;
  typedef arangodb::dbinterface::Server Server;
  DbTest();
  virtual ~DbTest();

 protected:
  void test1();

 private:
  void generalTest(const Connection::VPack (DbTest::*fn)(), uint64_t code);
  void createTest();
  void deleteTest();
  const Connection::VPack createDatabase();
  const Connection::VPack deleteDatabase();

  Server::SPtr _pSrv;
  Database::SPtr _pDb;
  Connection::SPtr _pCon;
};

inline DbTest::~DbTest() {}

#endif  // DBTEST_H
