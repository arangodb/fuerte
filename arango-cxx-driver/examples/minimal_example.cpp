
#include "gtest/gtest.h"
#include <fuerte/Server.h>
#include <fuerte/Database.h>
#include <fuerte/Collection.h>
#include <fuerte/Document.h>
#include <velocypack/Slice.h>
#include <velocypack/Builder.h>
#include <string>
#include <utility>
#include <fuerte/database.h>
#include <fuerte/connection.h>

int main(){
    //init
  if(false) {
    using namespace arangodb::dbinterface;
    auto server = std::make_shared<Server>("tcp://localhost:8529");
    auto db = std::make_shared<Database>(server, "test_database");
    auto coll = std::make_shared<Collection>(db, "my_collection");
    auto conn = server->makeConnection();
    conn->setAsynchronous(false);

    //create data
    arangodb::velocypack::Builder builder;
    builder.openObject();
    builder.add("_id",arangodb::velocypack::Value("wauuuu"));
    builder.close();

    //add document
    Document{}.create(coll,conn,builder.steal());

    //run query
    conn->setPostReq();
    conn->run();

    //inspect result
    Connection::VPack res = conn->result();
  } else {
    using namespace arangocxx;
    auto conn = ConnectionBuilder().async(false).user("hund").password("arfarf").connect();
    auto db = conn->getDatabase("fopples");

  }



}
