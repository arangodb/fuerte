#pragma once

#include "message.h"
namespace arangodb { namespace fuerte { inline namespace v1 {

using MessageCallback = std::function<void(std::unique_ptr<Request>, std::unique_ptr<Response>)>;
using Error = std::uint32_t;
using ErrorCallback = std::function<void(Error, std::unique_ptr<Request>, std::unique_ptr<Response>)>;


class ConnectionImplInterface {
  //interface class used in connection
public:
  ConnectionImplInterface(){};

  virtual Message sendMessage(std::unique_ptr<Message>) = 0;
  virtual void sendMessage(std::unique_ptr<Message>, ErrorCallback, MessageCallback) = 0;
};


}}}
