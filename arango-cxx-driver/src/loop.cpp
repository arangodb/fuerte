////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Christoph Uhde
////////////////////////////////////////////////////////////////////////////////

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
      catch (std::exception const& e)
      {
        FUERTE_LOG_ERROR << e.what() << std::endl; //return to loop
        _service->reset();
      }
      catch(...){
        FUERTE_LOG_ERROR << "unknown exception";
        _service->reset();
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
    _service->reset();
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

boost::asio::io_service* Loop::getIoService(){
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
