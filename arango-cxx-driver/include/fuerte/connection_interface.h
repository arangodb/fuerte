#pragma once

#include "message.h"
namespace arangodb { namespace fuerte { inline namespace v1 {

class ConnectionInterface {
  //interface class used in connection
public:
  ConnectionInterface(){};

  virtual Message sendRequest(std::unique_ptr<Request>) = 0;
  virtual void sendRequest(std::unique_ptr<Request>, OnErrorCallback, OnSuccessCallback) = 0;
  virtual void start() = 0;
};


}}}
