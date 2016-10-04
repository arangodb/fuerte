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

#ifndef FUERTE_HEADERSINGLE

#define FUERTE_HEADERSINGLE

#include "HeaderCommon.h"

namespace arangodb {

namespace dbinterface {

namespace Header {

class Single : public Common {
 public:
  //  Base class must have a virtual destructor
  virtual ~Single();
  Single();
  Single(const uint8_t* ptr);
  Single(MsgId id, SzChunk szChnk);
  enum { HeaderSize = Common::HeaderSize };

 protected:
  // Multi chunk constructor
  Single(ChunkInfo noChunks, MsgId msgId, SzChunk szChunk);

 public:
  ChunkInfo noChunks() const;
  ChunkInfo chunkNo() const override;
  bool bFirstChunk() const override;
  bool bSingleChunk() const override;
};

// Multi chunk constructor
inline Single::Single(ChunkInfo noChunks, MsgId msgId, SzChunk szChunk)
    : Common(noChunks, msgId, szChunk) {}

inline Single::Single(MsgId id, SzChunk szChnk) : Common{id, szChnk} {}

inline Single::Single() : Common{} {}

inline Single::~Single() {}

inline Single::Single(const uint8_t* ptr) : Common{ptr} {}

inline bool Single::bFirstChunk() const { return true; }

inline bool Single::bSingleChunk() const { return true; }

inline Common::ChunkInfo Single::chunkNo() const { return 0; }

inline Common::ChunkInfo Single::noChunks() const { return 1; }
}
}
}

#endif
