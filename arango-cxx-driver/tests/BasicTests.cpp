#include "gtest/gtest.h"
#include <fuerte/Server.h>
#include <fuerte/Database.h>
#include <velocypack/Slice.h>
#include <string>
#include <utility>

namespace BasicTest {
    typedef arangodb::dbinterface::Server Server;
    typedef arangodb::dbinterface::Database Database;
    typedef arangodb::dbinterface::Connection ConnectionBase;
    typedef arangodb::velocypack::Slice Slice;

    namespace velocypack = arangodb::velocypack;

    inline std::string const& hostUrl(){
        static const std::string url = "http://127.0.0.1:8529";
        return url;
    }

    inline std::string const& dbVersion(){
        static const std::string ver = "2.8.2";
        return ver;
    }

    class ConnectionFixture : public testing::Test {
    public:
        typedef arangodb::dbinterface::Server Server;
        typedef arangodb::dbinterface::Connection Connection;
        ConnectionFixture()
            :_pSrv{std::make_shared<Server>(hostUrl())}
            ,_pCon{_pSrv->makeConnection()}
            {}
        virtual ~ConnectionFixture(){};

    protected:
        ConnectionFixture::Connection::VPack getDbVersion() {
          Server& srv = *_pSrv;
          srv.version(_pCon);
          _pCon->run();
          return _pCon->result();
        }

    private:
        Server::SPtr _pSrv;
        Connection::SPtr _pCon;
    };

    TEST_F(ConnectionFixture, NEW_version) {
      typedef velocypack::Slice Slice;
      typedef velocypack::ValueType ValueType;

      Connection::VPack res = getDbVersion();
      Slice retSlice{res->data()};
      Slice slice = retSlice.get("version");
      if (slice.type() == ValueType::String) {
        std::string dbVersion = BasicTest::dbVersion();
        if (dbVersion != "*") {
          EXPECT_EQ(dbVersion, slice.copyString());
        }
        return;
      }
      slice = retSlice.get("errorMessage");
      if (slice.type() == ValueType::String) {
        ADD_FAILURE() << slice.copyString();
        return;
      }
      ADD_FAILURE() << "Unexpected failure" << slice.toJson();
    }


    class DataBaseFixture : public testing::Test {
    public:
        typedef arangodb::dbinterface::Server Server;
        typedef arangodb::dbinterface::Connection Connection;
        DataBaseFixture()
            :_pSrv{std::make_shared<Server>(hostUrl())}
            ,_pDb{std::make_shared<Database>(_pSrv, std::string{"Test"})}
            ,_pCon{_pSrv->makeConnection()}
            {}

        virtual ~DataBaseFixture(){};

        void generalTest(Connection::VPack (DataBaseFixture::*fn)(),uint64_t code) {
            typedef velocypack::Slice Slice;
            typedef velocypack::ValueType ValueType;
            typedef arangodb::dbinterface::Database Database;
            const Connection::VPack res = (this->*fn)();
            Slice retSlice{res->data()};
            Slice slice = retSlice.get("result");
            if (slice.type() == ValueType::Bool) {
              EXPECT_EQ(true, slice.getBool());
            }
            slice = retSlice.get("code");
            if (slice.type() == ValueType::UInt) {
              EXPECT_EQ(code, slice.getUInt());
              return;
            }
            slice = retSlice.get("errorMessage");
            if (slice.type() == ValueType::String) {
              ADD_FAILURE() << slice.copyString();
              return;
            }
            ADD_FAILURE() << "Unexpected failure";
        }
        Connection::VPack createDatabase() {
          Database& db = *_pDb;
          db.create(_pCon);
          _pCon->run();
          return _pCon->result();
        }
        Connection::VPack deleteDatabase() {
          Database& db = *_pDb;
          db.remove(_pCon);
          _pCon->run();
          return _pCon->result();
        }

        Server::SPtr _pSrv;
        Database::SPtr _pDb;
        Connection::SPtr _pCon;
    };

    TEST_F(DataBaseFixture, conneciton_create_delete) {
        SCOPED_TRACE("Create");
        generalTest(&DataBaseFixture::createDatabase, 201);
        SCOPED_TRACE("Delete");
        generalTest(&DataBaseFixture::deleteDatabase, 200);
    }
}
