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

#include <arangodbcpp/Version.h>
#include <arangodbcpp/fuerte_version.h>
#include <sstream>

namespace arangodb {

namespace dbinterface {

const uint16_t Version::_major = (VERSION_MAJOR);
const uint16_t Version::_minor = (VERSION_MINOR);
const uint16_t Version::_patch = (VERSION_PATCH);

const std::string Version::_appName{"arangodbcpp"};

int Version::compare(uint16_t major, uint16_t minor, uint16_t patch) {
  if (_major != major) {
    return _major < major ? -1 : 1;
  }
  if (_minor != minor) {
    return _minor < minor ? -1 : 1;
  }
  if (_patch != patch) {
    return _patch < patch ? -1 : 1;
  }
  return 0;
}

int Version::compare(uint16_t major, uint16_t minor) {
  if (_major != major) {
    return _major < major ? -1 : 1;
  }
  if (_minor != minor) {
    return _minor < minor ? -1 : 1;
  }
  return 0;
}

std::string Version::version() { return std::string(VERSION_STRING); }
}
}
