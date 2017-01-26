#pragma once
#include <string>
#include <vector>
#include <fuerte/types.h>

namespace arangodb { namespace fuerte { inline namespace v1 {

std::string to_string(VSlice const& slice);
std::string to_string(std::vector<VSlice> const& payload);

namespace http {
  std::string urlDecode(std::string const& str);
  std::string urlEncode(char const* src, size_t const len);
  std::string urlEncode(char const* src);

  inline std::string urlEncode(std::string const& str) {
    return urlEncode(str.c_str(), str.size());
  }
}
}}}
