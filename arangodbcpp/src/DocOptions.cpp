#include "arangodbcpp/DocOptions.h"

namespace arangodb {

namespace dbinterface {

DocOptions::DocOptions(const uint16_t opts, const std::string& tag)
    : _eTag{tag}, _opts{opts} {}

DocOptions::DocOptions(const uint16_t opts, std::string&& tag)
    : _eTag{tag}, _opts{opts} {}
}
}
