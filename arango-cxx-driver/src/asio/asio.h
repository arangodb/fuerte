// internal header
#pragma once
#ifndef ARANGO_CXX_DRIVER_ASIO
#define ARANGO_CXX_DRIVER_ASIO

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

namespace arangodb { namespace fuerte { inline namespace v1 {
  class LoopProvider;
}}}
namespace arangodb { namespace fuerte { inline namespace v1 { namespace asio {
class Loop{
  friend class ::arangodb::fuerte::v1::LoopProvider;
public:
  Loop()
    :_serviceSharedPtr(new ::boost::asio::io_service)
    ,_service(_serviceSharedPtr.get())
    ,_sealed(false)
    ,_owning(true)
    ,_running(false)
    {}

  bool run(){
    if(!_running && _service){
      _running = true;
      _sealed = true;
      _work = std::make_shared<::boost::asio::io_service::work>(*_service);
      while(_work){
        try
        {
          _service->run();
          break; // run() exited normally
        }
        catch (std::exception& e)
        {
          // Deal with exception as appropriate.
        }
      }
      return true;
    }
    return false;
  };

  void poll(){
    _sealed = true;
    _service->poll();
  }

  void ask_to_stop(){
    if(!_running) { return; }
    _work.reset();
    _running = false;
  }

private:
  void setIoService(::boost::asio::io_service * service){
    if(_sealed){ return; }
    _owning = false;
    _serviceSharedPtr = nullptr;
    _service = service;
  }

  void setIoServiceTakeOwnership(::boost::asio::io_service* service){
    if(_sealed){ return; }
    _owning = true;
    _serviceSharedPtr.reset(service);
    _service = service;
  }

  boost::asio::io_service* getIoService(){
    _sealed = true;
    return _service;
  }


private:
  std::shared_ptr<::boost::asio::io_service> _serviceSharedPtr;
  ::boost::asio::io_service* _service;
  std::shared_ptr<::boost::asio::io_service::work> _work;
  bool _owning;
  bool _sealed;
  bool _running;

};


}}}}
#endif


