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
/// @author Jan Christoph Uhde
////////////////////////////////////////////////////////////////////////////////
#include "test_main.h"

#include "vst.h"
#include "Basics/Format.h"

namespace fu = ::arangodb::fuerte;

// testsuite for VST 1.1

TEST(VSTParser, ChunkHeader) {
  std::string const length("\x1C\0\0\0", 4); // 24 + 4 bytes length
  std::string const chunkX("\x03\0\0\0", 4); // 0 chunks
  std::string const mid("\x01\0\0\0\0\0\0\0", 8); // messageId 1
  std::string const mLength("\x04\0\0\0\0\0\0\0", 8); // messageLength
  std::string const data("\x0a\x0b\x0c\x0d", 4); // messageId 1

  std::string chunk = length + chunkX + mid + mLength + data;
  ASSERT_EQ(chunk.size(), 28);

  uint8_t const* ptr = reinterpret_cast<uint8_t const*>(chunk.c_str());
  size_t chunkLength = fu::vst::parser::isChunkComplete(ptr, chunk.size());
  ASSERT_EQ(chunkLength, 28);
  
  fu::vst::ChunkHeader header = fu::vst::parser::readChunkHeaderVST1_1(ptr);
  ASSERT_EQ(header.chunkLength(), 28);
  ASSERT_EQ(header.messageID(), 1);
  ASSERT_EQ(header.messageLength(), 4);
  ASSERT_TRUE(header.isFirst());
  ASSERT_EQ(header.index(), 0);
  ASSERT_EQ(header.numberOfChunks(), 1);
  ASSERT_EQ(header._data.size(), 4);
  
  ptr = reinterpret_cast<uint8_t const*>(header._data.data());
  uint32_t val = arangodb::basics::uintFromPersistentLittleEndian<uint32_t>(ptr);
  ASSERT_EQ(val, static_cast<uint32_t>(0x0d0c0b0a));
}

TEST(VSTParser, MultiChunk){
  ASSERT_TRUE(true); //TODO -- DELETE
  
  
}
