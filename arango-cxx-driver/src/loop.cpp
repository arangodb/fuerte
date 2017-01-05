#include "asio.h"
#include <fuerte/loop.h>
#include "HttpCommunicator.h"
namespace arangodb { namespace fuerte { inline namespace v1 {
  LoopProvider::LoopProvider()
    :_asioLoop(new asio::Loop{})
    ,_httpLoop(new http::HttpCommunicator())
    {}

  void LoopProvider::setAsioLoop(::boost::asio::io_service* service){
    _asioLoop->setIoService(service);
  }

  void runHttpLoopInThisThread(){
    auto loop = LoopProvider::getProvider().getHttpLoop();
    if(loop){
      while (true) {
        int left = loop->workOnce();
        if (left == 0) { break; }
          loop->wait();
        }
    }
  }

}}}
