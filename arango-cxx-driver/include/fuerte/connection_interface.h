#pragma once

#include "message.h"
namespace arangodb { namespace fuerte { inline namespace v1 {

class ConnectionInterface {
  // this class is a member of the connection class and it provides
  // an interface so that the curl and asio implementaion can be hideen
  // via pointer to implementaton
public:
  ConnectionInterface(){};
  virtual std::unique_ptr<Response> sendRequest(std::unique_ptr<Request>) = 0;
  virtual MessageID sendRequest(std::unique_ptr<Request>, OnErrorCallback, OnSuccessCallback) = 0;
  virtual void start(){};
};

}}}
