#pragma once

#include <fuerte/common_types.h>

namespace arangodb { namespace fuerte { inline namespace v1 { namespace asio {
  struct Options {
    double requestTimeout = 120.0;
    double connectionTimeout = 2.0;
  };

  class Task {
    std::unique_ptr<Request> _request;
    Callbacks _callbacks;
    Options _options;
    Ticket _ticketId;
  };






}}}}
