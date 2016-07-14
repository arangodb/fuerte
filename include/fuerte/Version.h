////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
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
/// @author John Bufton
////////////////////////////////////////////////////////////////////////////////

#ifndef FUERTE_VERSION_H
#define FUERTE_VERSION_H 1

#include <string>

namespace arangodb {
namespace dbinterface {
class Version {
 public:
  Version() = delete;
  Version(const Version&) = delete;
  Version(Version&&) = delete;

  static uint16_t major();
  static uint16_t minor();
  static uint16_t patch();
  static std::string appName();
  static std::string version();

  static int compare(uint16_t major, uint16_t minor, uint16_t patch);
  static int compare(uint16_t major, uint16_t minor);

 private:
  static const uint16_t _major;
  static const uint16_t _minor;
  static const uint16_t _patch;
  static const std::string _appName;
};

inline uint16_t Version::major() { return _major; }

inline uint16_t Version::minor() { return _minor; }

inline uint16_t Version::patch() { return _patch; }

inline std::string Version::appName() { return _appName; }
}
}

#endif
