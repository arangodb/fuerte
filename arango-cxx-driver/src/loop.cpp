#include "asio.h"
#include "loop.h"
namespace arangodb { namespace fuerte { inline namespace v1 {
  LoopProvider::LoopProvider()
    :_asioLoop(new asio::Loop{})
    {}

  void LoopProvider::setAsioLoop(::boost::asio::io_service* service){
    _asioLoop->setIoService(service);
  }

}}}
