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

GlobalService::GlobalService() { 
  FUERTE_LOG_DEBUG << "GlobalService init" << std::endl;
  ::curl_global_init(CURL_GLOBAL_ALL); 
}
GlobalService::~GlobalService() { 
  FUERTE_LOG_DEBUG << "GlobalService cleanup" << std::endl;
  ::curl_global_cleanup(); 
}

EventLoopService::EventLoopService(unsigned int threadCount)
    : EventLoopService(threadCount, std::make_shared<asio_io_service>()) {}

// Initialize an EventLoopService with a given number of threads and a given
// io_service.
EventLoopService::EventLoopService(
    unsigned int threadCount,
    const std::shared_ptr<asio_io_service>& io_service)
    : EventLoopService(threadCount, io_service,
                       std::make_shared<http::HttpCommunicator>(io_service)) {}

// run is called for each thread. It calls io_service.run() and
// invokes the curl handlers.
// You only need to invoke this if you want a custom event loop service.
void EventLoopService::run() {
  try {
    io_service_->run();
  } catch (std::exception const& ex) {
    handleRunException(ex);
  }
}


// io_service::work is a inner class and we need to declare it forward
/*struct Work {
  Work(boost::asio::io_service& serv):_work(serv){}
  ::boost::asio::io_service::work _work;
};*/


// Loop ///////////////////////////////////////////////////////////////
/*
Loop::Loop()
  :_serviceSharedPtr(new ::boost::asio::io_service)
  ,_service(_serviceSharedPtr.get())
  ,_sealed(false)
  ,_owning(true)
  ,_running(false)
  ,_singleRunMode(false)
  {}

// Run the eventloop. Only returns when all work is done or the eventloop is stopped.
// There is no need to reset the eventloop sfter this call, unless an exception is thrown.
void Loop::run_ready(){
  _sealed = true;
  if(!_owning){
    return;
  }
  FUERTE_LOG_DEBUG << "fuerte - asio: run() [to completion (single thread mode)]" << std::endl;
  _singleRunMode=true;
  try{
    std::cout.flush();
    _service->run();
  } catch (std::exception const& e) {
    FUERTE_LOG_ERROR << "error during run: " << e.what() << std::endl;
    throw e;
  } catch (...) {
    FUERTE_LOG_ERROR << "error during run: " << std::endl;
    throw;
  }
  _service->reset();
  _singleRunMode=false;
}

// Run all handlers that are ready to run without blocking.
// Only returns when there are no more ready handlers, or or the eventloop is stopped.
void Loop::poll(){
  _sealed = true;
  if(!_owning){
    return;
  }
#if ENABLE_FUERTE_LOG_CALLBACKS > 0
  FUERTE_LOG_DEBUG << "fuerte - asio: poll()" << std::endl;
#endif
  FUERTE_LOG_CALLBACKS << "P" << std::endl;
  try{
    _service->poll();
  } catch (std::exception const& e) {
    FUERTE_LOG_ERROR << "error during poll: " << e.what() << std::endl;
    throw e;
  } catch (...) {
    FUERTE_LOG_ERROR << "error during poll: " << std::endl;
    throw;
  }
}

// Reset the eventloop, so run can function again.
void Loop::reset(){
  _service->reset();
}

void Loop::direct_poll(){
  _service->run();
}

// Run the eventloop. Only return when all work is done or the eventloop is stopped.
void Loop::direct_run(){
  _service->run();
}

// Stop the eventloop as soon as possible.
void Loop::direct_stop(){
  _service->stop();
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

void LoopProvider::run(){
  static std::mutex mutex;

  if(_asioLoop){
    _asioLoop->run_ready();  //runs unitl all work is done
  }

  //poll http -- spins busy for http
  if(_httpLoop && _httpLoop->used() && mutex.try_lock()){
    while (true) {
      int left = _httpLoop->workOnce(); // check if there is still somehting to do
      if (left == 0) { break; }
      _httpLoop->wait(); // work happens here
    }
    mutex.unlock();
  }
}

void LoopProvider::poll(){
  static std::mutex mutex;
  //poll asio
  if(_asioLoop){
    _asioLoop->poll();  //polls until io_service has no further tasks
  }

  //poll http
  if(_httpLoop && mutex.try_lock()){
    int left = _httpLoop->workOnce();
    mutex.unlock();
  }
}

void LoopProvider::resetIoService(){
  if(_asioLoop){
    _asioLoop->reset();
  }
}



//void Loop::run(){
//  if(!_running && _service && _owning){
//    _running = true;
//    _sealed = true;
//    _work = std::make_shared<Work>(*_service);
//    while(_work){
//      try
//      {
//        _service->run();
//        break; // run() exited normally
//      }
//      catch (std::exception const& e)
//      {
//        FUERTE_LOG_ERROR << e.what() << std::endl; //return to loop
//        _service->reset();
//      }
//      catch(...){
//        FUERTE_LOG_ERROR << "unknown exception";
//        _service->reset();
//      }
//    }
//    return true;
//  }
//  return false;
//};
// void Loop::ask_to_stop(){
//   if(!_running || !_owning) { return; }
//   _work.reset();
//   _running = false;
// }
//
*/

}}}
