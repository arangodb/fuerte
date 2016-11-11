#pragma once

#include "request.h"
namespace arangodb { namespace rest { inline namespace v2 {

using RequestCallback = std::function<void(Request)>;
using Error = std::size_t;
using ErrorCallback = std::function<void(Error)>;


class RealConnection {
  //interface class used in connection
  RealConnection(){};

  virtual Request sendRequest(Request) = 0;
  virtual void sendRequest(Request, ErrorCallback, RequestCallback) = 0;
};


}}}
