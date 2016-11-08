#pragma once

#include <string>
#include <vector>
#include <map>

#include <velocypack/Builder.h>

namespace arangodb { namespace rest { inline namespace v2 {

  class Request {
    private:
      std::map<std::string, std::string> _header;


  };

}}}
