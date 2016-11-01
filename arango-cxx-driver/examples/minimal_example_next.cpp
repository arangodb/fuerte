namespace arangocxx {
  class Database;
}

#include <fuerte/next/connection.h>
#include <fuerte/next/collection.h>
#include <fuerte/next/database.h>

int main(){
    using namespace arangocxx;
    auto conn = ConnectionBuilder().host("http://localhost:8529")
//                                   .async(true)
                                   .user("hund")
                                   .password("arfarf")
                                   .connect();
    auto coll = conn->getDatabase("fopples")->getCollection("plastic");
    coll->insert("fasdfasdfasdfas");
}
