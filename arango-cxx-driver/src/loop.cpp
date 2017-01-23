#include <memory>
#include <boost/asio/io_service.hpp>
#include <fuerte/loop.h>
#include "HttpCommunicator.h"

namespace arangodb { namespace fuerte { inline namespace v1 {

static VpackInit init;

class LoopProvider;
class VstConnection;

// io_service::work is a inner class and we need to declare it forward
struct Work {
  Work(boost::asio::io_service& serv):_work(serv){}
  ::boost::asio::io_service::work _work;
};


// Loop ///////////////////////////////////////////////////////////////

Loop::Loop()
  :_serviceSharedPtr(new ::boost::asio::io_service)
  ,_service(_serviceSharedPtr.get())
  ,_sealed(false)
  ,_owning(true)
  ,_running(false)
  ,_pollMode(false)
  {}

bool Loop::run(){
  if(!_running && _service && _owning){
    _running = true;
    _sealed = true;
    _work = std::make_shared<Work>(*_service);
    while(_work){
      try
      {
        _service->run();
        break; // run() exited normally
      }
      catch (std::exception& e)
      {
        FUERTE_LOG_ERROR << e.what() << std::endl; //return to loop
      }
      catch(...){
        FUERTE_LOG_ERROR << "unknown execeotion: terminating";
        std::abort();
      }
    }
    return true;
  }
  return false;
};

void Loop::poll(bool block){
  _sealed = true;
  if(!_owning){
    return;
  }

  if(block){
    _pollMode=true;
    _service->run();
    _pollMode=false;
  } else {
    _service->poll();
  }
}

void Loop::ask_to_stop(){
  if(!_running || !_owning) { return; }
  _work.reset();
  _running = false;
}

void Loop::setIoService(::boost::asio::io_service * service){
  if(_sealed){ return; }
  _owning = false;
  _serviceSharedPtr = nullptr;
  _service = service;
}

void Loop::setIoServiceTakeOwnership(::boost::asio::io_service* service){
  if(_sealed){ return; }
  _owning = true;
  _serviceSharedPtr.reset(service);
  _service = service;
}

void* Loop::getIoService(){
  _sealed = true;
  return _service;
}

// LoopProvider ///////////////////////////////////////////////////////////////

LoopProvider::LoopProvider()
  :_asioLoop(new Loop{})
  ,_httpLoop(new http::HttpCommunicator())
  {}

void LoopProvider::setAsioService(::boost::asio::io_service* service, bool running){
  if (_asioLoop->_sealed) {
    throw std::logic_error("you try to modify a loop that is already in use");
  } else {
    _asioLoop->setIoService(service);
  }
}

std::shared_ptr<Loop> LoopProvider::getAsioLoop(){
  _asioLoop->_sealed = true;
  return _asioLoop;
}

void* LoopProvider::getAsioIoService(){
  return _asioLoop->getIoService();
}

bool LoopProvider::runAsio(){
  return _asioLoop->run();
}

void LoopProvider::pollAsio(bool block){
  _asioLoop->poll(block);
}

bool LoopProvider::isAsioPolling(){
  return _asioLoop->_pollMode;
}


void LoopProvider::poll(bool block){
  //poll asio
  if(_asioLoop){
    pollAsio(block);  //polls until io_service has no further tasks
  }

  //poll http -- spins busy for http
  if(_httpLoop){
    while (true) {
      int left = _httpLoop->workOnce(); // check if there is still somehting to do
      if (left == 0) { break; }
      _httpLoop->wait(); // work happens here
    }
  }
}

}}}
