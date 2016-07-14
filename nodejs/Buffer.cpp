#include <cstring>

#include "Buffer.h"

//
//	curlpp callback function that receives the data returned
//	from the operation performed into the default write buffer
//
size_t Buffer::WriteMemoryCallback(const char* ptr, size_t size, size_t nmemb) {
  size_t realsize = size * nmemb;
  if (realsize != 0) {
    size_t offset = _buf.size();
    _buf.resize(offset + realsize);
    memcpy(&_buf[offset], ptr, realsize);
  }
  return realsize;
}

//
// Converts the received buffer to a std:string object
//
std::string Buffer::result() const {
  if (!_buf.empty()) {
    return std::string{&_buf[0], _buf.size()};
  }
  return std::string{};
}
