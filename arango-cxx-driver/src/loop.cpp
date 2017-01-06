#include "asio/asio.h"
#include <fuerte/loop.h>
#include "HttpCommunicator.h"

namespace arangodb { namespace fuerte { inline namespace v1 {

  static VpackInit init;

  LoopProvider::LoopProvider()
    :_asioLoop(new asio::Loop{})
    ,_httpLoop(new http::HttpCommunicator())
    ,_sealed(false)
    {}

  void LoopProvider::setAsioLoop(::boost::asio::io_service* service){
    if (_sealed) {
      throw std::logic_error("you try to modify a loop that is already in use");
    } else {
      _asioLoop->setIoService(service);
    }
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
