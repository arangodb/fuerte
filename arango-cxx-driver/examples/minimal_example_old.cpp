#include <fuerte/old/Server.h>
#include <fuerte/old/Database.h>
#include <fuerte/old/Collection.h>
#include <fuerte/old/Connection.h>
#include <fuerte/old/Document.h>
#include <velocypack/Slice.h>
#include <velocypack/Builder.h>
#include <string>
#include <utility>

int main(){
    //init
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
}
