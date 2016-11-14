#include "asio.h"
#include "loop.h"
namespace arangodb { namespace rest { inline namespace v2 {
  LoopProvider::LoopProvider()
    :_asioLoop(new asio::Loop{})
    {}

  void LoopProvider::setAsioLoop(::boost::asio::io_service* service){
    _asioLoop->setIoService(service);
  }

}}}
