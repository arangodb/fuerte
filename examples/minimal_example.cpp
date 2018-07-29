#include <fuerte/fuerte.h>

int main(){
    using namespace arangodb::fuerte;
    EventLoopService eventLoopService;
    auto conn = ConnectionBuilder().endpoint("http://localhost:8529")
                                   .user("hund")
                                   .password("arfarf")
                                   .connect(eventLoopService);

    //auto coll = conn->getDatabase("fopples")->getCollection("plastic");
    //coll->insert("coca cola standard fopple");
}
