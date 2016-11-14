#pragma once

#include "request.h"
namespace arangodb { namespace rest { inline namespace v2 {

using RequestCallback = std::function<void(Request)>;
using Error = std::size_t;
using ErrorCallback = std::function<void(Error)>;


class ConnectionImplInterface {
  //interface class used in connection
public:
  ConnectionImplInterface(){};

  virtual Request sendRequest(Request) = 0;
  virtual void sendRequest(Request, ErrorCallback, RequestCallback) = 0;
};


}}}
