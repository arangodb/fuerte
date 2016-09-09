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

#include "../include/fuerte/HeaderCommon.h"

namespace arangodb {

namespace dbinterface {

namespace Header {

Common::Common(const uint8_t* ptr) {
  _szChnk = *reinterpret_cast<const SzChunk*>(ptr + IdxSzChunk);
  _chnkNo = *reinterpret_cast<const SzChunk*>(ptr + IdxChunkInfo);
  _msgId = *reinterpret_cast<const SzMsg*>(ptr + IdxMsgId);
}

//
//  Constructor for header a single chunk message
//
Common::Common(MsgId id, SzChunk sz)
    : _szChnk(sz), _chnkNo(SingleChunk), _msgId(id) {}

//
//  Constructor for header for a multiple chunk message
//
Common::Common(ChunkInfo nChks, MsgId id, SzChunk sz)
    : _szChnk(sz), _chnkNo(1 + (nChks << 1)), _msgId(id) {}

//
//  Constructor for header for an extra chunk
//
Common::Common(MsgId id, SzChunk sz, ChunkInfo chnkNo)
    : _szChnk(sz), _chnkNo(chnkNo << 1), _msgId(id) {}

void Common::headerOut(uint8_t* ptr) const {
  *reinterpret_cast<SzChunk*>(ptr + IdxSzChunk) = _szChnk;
  *reinterpret_cast<SzChunk*>(ptr + IdxChunkInfo) = _chnkNo;
  *reinterpret_cast<SzMsg*>(ptr + IdxMsgId) = _msgId;
}

void Common::headerIn(const uint8_t* ptr) {
  _szChnk = *reinterpret_cast<const SzChunk*>(ptr + IdxSzChunk);
  _chnkNo = *reinterpret_cast<const ChunkInfo*>(ptr + IdxChunkInfo);
  _msgId = *reinterpret_cast<const SzMsg*>(ptr + IdxMsgId);
}
}
}
}
