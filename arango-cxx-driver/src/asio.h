// internal header
#pragma once
#ifndef ARANGO_CXX_DRIVER_ASIO
#define ARANGO_CXX_DRIVER_ASIO

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

namespace arangodb { namespace rest { namespace asio { inline namespace v2 {
class Loop{
  ::boost::asio::io_service service;
};


}}}}
#endif


