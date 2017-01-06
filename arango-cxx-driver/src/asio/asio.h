// internal header
#pragma once
#ifndef ARANGO_CXX_DRIVER_ASIO
#define ARANGO_CXX_DRIVER_ASIO

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

namespace arangodb { namespace fuerte { inline namespace v1 { namespace asio {
class Loop{
public:
  Loop()
    :_serviceSharedPtr(new ::boost::asio::io_service)
    ,_service(_serviceSharedPtr.get())
    {}

  void setIoService(::boost::asio::io_service * service){
    _serviceSharedPtr = nullptr;
    _service = service;
  }

  void setIoServiceTakeOwnership(::boost::asio::io_service* service){
    _serviceSharedPtr.reset(service);
    _service = service;
  }

  boost::asio::io_service* getIoService(){
    return _service;
  }

private:
  std::shared_ptr<::boost::asio::io_service> _serviceSharedPtr;
  ::boost::asio::io_service* _service;

};


}}}}
#endif


