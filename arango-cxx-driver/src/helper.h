#pragma once
#include <string>

namespace arangodb { namespace fuerte { inline namespace v1 {

std::string urlEncode(char const* src, size_t const len);
std::string urlEncode(char const* src);

inline std::string urlEncode(std::string const& str) {
  return urlEncode(str.c_str(), str.size());
}

std::string urlDecode(std::string const& str);

}}}
