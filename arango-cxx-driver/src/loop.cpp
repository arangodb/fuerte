#include "asio.h"
#include <fuerte/loop.h>
#include "HttpCommunicator.h"

namespace arangodb { namespace fuerte { inline namespace v1 {

static VpackInit init;

LoopProvider::LoopProvider()
  :_asioLoop(new asio::Loop{})
  ,_httpLoop(new http::HttpCommunicator())
  {}

void LoopProvider::setAsioService(::boost::asio::io_service* service, bool running){
  if (_asioLoop->_sealed) {
    throw std::logic_error("you try to modify a loop that is already in use");
  } else {
    _asioLoop->setIoService(service);
  }
}

std::shared_ptr<asio::Loop> LoopProvider::getAsioLoop(){
  _asioLoop->_sealed = true;
  return _asioLoop;
}

void* LoopProvider::getAsioIoService(){
  return _asioLoop->getIoService();
}

bool LoopProvider::runAsio(){
  return _asioLoop->run();
}

void LoopProvider::pollAsio(){
  _asioLoop->poll();
}

void LoopProvider::poll(){
  //poll asio
  pollAsio();

  //poll http
  if(_httpLoop){
    while (true) {
      int left = _httpLoop->workOnce(); // check if there is still somehting to do
      if (left == 0) { break; }
      _httpLoop->wait(); // work happens here
    }
  }
}

}}}
