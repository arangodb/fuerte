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

#ifndef FUERTE_HEADERCOMMON

#define FUERTE_HEADERCOMMON

#include <stdint.h>
#include <vector>

namespace arangodb {

namespace dbinterface {

namespace Header {

class Common {
 public:
  typedef uint64_t MsgId;
  typedef uint64_t SzMsg;
  typedef uint32_t SzChunk;
  typedef uint32_t ChunkInfo;
  typedef std::vector<uint8_t> Chunk;
  typedef std::vector<Chunk> Chunks;
  Common();
  Common(const uint8_t* ptr);
  Common(MsgId id, SzChunk sz, ChunkInfo chnkNo);
  //  Base class must have a virtual destructor
  virtual ~Common();

 protected:
  //  Single chunk
  Common(MsgId id, SzChunk sz);
  //  Multi chunk
  Common(ChunkInfo nChks, MsgId id, SzChunk sz);

 public:
  virtual void headerOut(uint8_t* ptr) const;
  virtual void headerIn(const uint8_t* ptr);
  SzChunk szChunk() const;
  virtual bool bFirstChunk() const;
  virtual bool bSingleChunk() const;
  virtual ChunkInfo chunkNo() const;
  virtual uint16_t szHeader() const;
  virtual SzMsg szChunks() const;

  static ChunkInfo chnkInfo(const uint8_t* ptr);
  static bool bFirstChunk(ChunkInfo info);
  static bool bSingleChunk(ChunkInfo info);
  enum : uint16_t { HeaderSize = 16 };

 protected:
  enum : uint16_t {
    SingleChunk = 3,
    IdxSzChunk = 0,
    IdxChunkInfo = sizeof(SzChunk),
    IdxMsgId = IdxChunkInfo + sizeof(ChunkInfo),
    IdxMsgSize = IdxMsgId + sizeof(MsgId)
  };

 private:
  SzChunk _szChnk;
  ChunkInfo _chnkNo;
  MsgId _msgId;
};

inline Common::Common() {}

inline Common::~Common() {}

inline Common::ChunkInfo Common::chnkInfo(const uint8_t* ptr) {
  return *reinterpret_cast<const ChunkInfo*>(ptr + IdxChunkInfo);
}

inline Common::SzChunk Common::szChunk() const { return _szChnk; }

inline Common::SzMsg Common::szChunks() const { return 0; }

inline bool Common::bFirstChunk(ChunkInfo info) { return (info & 0x1) != 0; }

inline bool Common::bFirstChunk() const { return false; }

inline bool Common::bSingleChunk(ChunkInfo info) { return info == SingleChunk; }

inline bool Common::bSingleChunk() const { return false; }

inline Common::ChunkInfo Header::Common::chunkNo() const {
  return _chnkNo >> 1;
}

inline uint16_t Common::szHeader() const { return HeaderSize; }
}
}
}

#endif
