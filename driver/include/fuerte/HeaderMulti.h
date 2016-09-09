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

#ifndef FUERTE_HEADERMULTI

#define FUERTE_HEADERMULTI

#include <limits>
#include "HeaderSingle.h"

namespace arangodb {

namespace dbinterface {

namespace Header {

class Multi : public Single {
 public:
  Multi();
  Multi(const uint8_t* ptr);
  Multi(ChunkInfo noChunks, MsgId msgId, SzChunk szChunk, SzMsg szMsg);
  uint16_t szHeader() const override;
  SzMsg szChunks() const override;
  bool bSingleChunk() const override;
  void headerOut(uint8_t* ptr) const override;

  enum : uint16_t { HeaderSize = 24 };
#define __TMP__ (std::numeric_limits<uint32_t>::max())
  enum : uint64_t {
    MaxSzMsg = static_cast<uint64_t>(__TMP__ - Common::HeaderSize) *
                   ((__TMP__ >> 1) - 1) +
               (__TMP__ - Multi::HeaderSize)
  };
#undef __TMP__
 private:
  MsgId _szMsg;
};

inline Multi::Multi() : Single{} {}

inline bool Multi::bSingleChunk() const { return false; }

inline uint16_t Multi::szHeader() const { return HeaderSize; }
}
}
}

#endif
