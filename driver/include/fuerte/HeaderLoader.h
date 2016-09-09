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

#ifndef FUERTE_HEADERLOADER
#define FUERTE_HEADERLOADER

#include "HeaderMulti.h"

namespace arangodb {

namespace dbinterface {

namespace Header {

class Loader {
  union {
    uint8_t _common[sizeof(Common)];
    uint8_t _single[sizeof(Single)];
    uint8_t _multi[sizeof(Multi)];
  } _hdr;

 public:
  Common& operator()(const uint8_t* ptr);
};
}
}
}

#endif
