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

#include "../include/fuerte/HeaderMulti.h"

namespace arangodb {

namespace dbinterface {

namespace Header {

Multi::Multi(ChunkInfo noChunks, MsgId msgId, SzChunk szChunk, SzMsg szMsg)
    : Single(noChunks, msgId, szChunk), _szMsg(szMsg) {}

Multi::Multi(const uint8_t* ptr) : Single(ptr) {
  _szMsg = *reinterpret_cast<const MsgId*>(ptr + IdxMsgSize);
}

void Multi::headerOut(uint8_t* ptr) const {
  Common::headerOut(ptr);
  *reinterpret_cast<MsgId*>(ptr + Common::HeaderSize) = _szMsg;
}

Common::SzMsg Multi::szChunks() const {
  MsgId res = noChunks() - 1;
  res *= Common::HeaderSize;
  res += HeaderSize;
  res + _szMsg;
  return res;
}
}
}
}
