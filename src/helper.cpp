////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016-2018 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Christoph Uhde
/// @author Dr. Frank Celler
////////////////////////////////////////////////////////////////////////////////
#include <fuerte/helper.h>
#include <string.h>
#include <sstream>
#include <stdexcept>

#include <velocypack/Iterator.h>

namespace arangodb { namespace fuerte { inline namespace v1 {
StringMap sliceToStringMap(VSlice const& slice) {
  StringMap rv;
  assert(slice.isObject());
  for (auto const& it : ::arangodb::velocypack::ObjectIterator(slice)) {
    rv.insert({it.key.copyString(), it.value.copyString()});
  }
  return rv;
}

std::string to_string(VSlice const& slice) {
  std::stringstream ss;
  try {
    std::string json = slice.toJson();
    ss << json << ", " << slice.byteSize() << ", " << json.length();
  } catch (std::exception& e) {
    ss << "slice to string failed with: " << e.what();
  }
  ss << std::endl;

  return ss.str();
}

std::string to_string(std::vector<VSlice> const& slices) {
  std::stringstream ss;
  if (!slices.empty()) {
    for (auto const& slice : slices) {
      ss << to_string(slice);
    }
  } else {
    ss << "empty";
  }
  ss << "\n";
  return ss.str();
}

// message is not const because message.slices is not
std::string to_string(Message& message) {
  std::stringstream ss;
  ss << "\n#### Message #####################################\n";
  ss << "Id:" << message.messageID << "\n";
  ss << "Header:\n";
  ss << to_string(message.header);
  ss << "\nBody:\n";
  if (message.contentType() == ContentType::VPack) {
    ss << to_string(message.slices());
  } else {
    ss << message.payloadAsString();
  }
  ss << "\n";
  ss << "##################################################\n";
  return ss.str();
}

// .............................................................................
// BASE64
// .............................................................................

char const* const BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string encodeBase64(std::string const& in) {
  unsigned char charArray3[3];
  unsigned char charArray4[4];

  std::string ret;
  ret.reserve((in.size() * 4 / 3) + 2);

  int i = 0;

  unsigned char const* bytesToEncode =
      reinterpret_cast<unsigned char const*>(in.c_str());
  size_t in_len = in.size();

  while (in_len--) {
    charArray3[i++] = *(bytesToEncode++);

    if (i == 3) {
      charArray4[0] = (charArray3[0] & 0xfc) >> 2;
      charArray4[1] =
          ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
      charArray4[2] =
          ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
      charArray4[3] = charArray3[2] & 0x3f;

      for (i = 0; i < 4; i++) {
        ret += BASE64_CHARS[charArray4[i]];
      }

      i = 0;
    }
  }

  if (i != 0) {
    for (int j = i; j < 3; j++) {
      charArray3[j] = '\0';
    }

    charArray4[0] = (charArray3[0] & 0xfc) >> 2;
    charArray4[1] =
        ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
    charArray4[2] =
        ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
    charArray4[3] = charArray3[2] & 0x3f;

    for (int j = 0; (j < i + 1); j++) {
      ret += BASE64_CHARS[charArray4[j]];
    }

    while ((i++ < 3)) {
      ret += '=';
    }
  }

  return ret;
}

std::string encodeBase64U(std::string const& in) {
  std::string encoded = encodeBase64(in);
  // replace '+', '/' with '-' and '_'
  std::replace(encoded.begin(), encoded.end(), '+', '-');
  std::replace(encoded.begin(), encoded.end(), '/', '_');
  return encoded;
}
}}}  // namespace arangodb::fuerte::v1
