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

TEST(VelocyStream_11, ChunkHeader) {
  std::string const length("\x1C\0\0\0", 4); // 24 + 4 bytes length
  std::string const chunkX("\x03\0\0\0", 4); // 1 chunk
  std::string const mid("\x01\0\0\0\0\0\0\0", 8); // messageId 1
  std::string const mLength("\x04\0\0\0\0\0\0\0", 8); // messageLength
  std::string const data("\x0a\x0b\x0c\x0d", 4); // messageId 1

  std::string chunk = length + chunkX + mid + mLength + data;
  ASSERT_EQ(chunk.size(), 28);

  uint8_t const* ptr = reinterpret_cast<uint8_t const*>(chunk.c_str());
  size_t chunkLength = fu::vst::parser::isChunkComplete(ptr, chunk.size());
  ASSERT_EQ(chunkLength, 28);
  
  auto resultChunk = fu::vst::parser::readChunkHeaderVST1_1(ptr);
  ASSERT_EQ(resultChunk.header.chunkLength(), 28);
  ASSERT_EQ(resultChunk.header.messageID(), 1);
  ASSERT_EQ(resultChunk.header.messageLength(), 4);
  ASSERT_TRUE(resultChunk.header.isFirst());
  ASSERT_EQ(resultChunk.header.index(), 0);
  ASSERT_EQ(resultChunk.header.numberOfChunks(), 1);
  ASSERT_EQ(resultChunk.body.size(), 4);
  
  ptr = reinterpret_cast<uint8_t const*>(resultChunk.body.data());
  uint32_t val = fu::basics::uintFromPersistentLittleEndian<uint32_t>(ptr);
  ASSERT_EQ(val, static_cast<uint32_t>(0x0d0c0b0a));
  
  arangodb::velocypack::Buffer<uint8_t> tmp;
  size_t t = resultChunk.header.writeHeaderToVST1_1(4, tmp);
  ASSERT_EQ(tmp.size(), fu::vst::maxChunkHeaderSize);
  ASSERT_EQ(t, fu::vst::maxChunkHeaderSize);
  ASSERT_LE(fu::vst::maxChunkHeaderSize, chunk.length());
  ASSERT_TRUE(memcmp(tmp.data(), chunk.data(), fu::vst::maxChunkHeaderSize) == 0);
}

TEST(VelocyStream_11, MultiChunk){
  std::string const length("\x1C\0\0\0", 4); // 24 + 4 bytes length
  std::string const chunkX_0("\x07\0\0\0", 4); // 3 chunks = ((0b11 << 1) | 1)
  std::string const chunkX_1("\x02\0\0\0", 4); // chunk 1 ((0b01 << 1) | 0)
  std::string const chunkX_2("\x04\0\0\0", 4); // chunk 2 ((0b10 << 1) | 0)
  std::string const mid("\x01\0\0\0\0\0\0\x01", 8); // messageId
  std::string const mLength("\x0C\0\0\0\0\0\0\0", 8); // messageLength
  std::string const data("\x0a\x0b\x0c\x0d", 4); // messageId 1
 
  std::string chunk0 = length + chunkX_0 + mid + mLength + data;
  std::string chunk1 = length + chunkX_1 + mid + mLength + data;
  std::string chunk2 = length + chunkX_2 + mid + mLength + data;

  ASSERT_EQ(chunk0.size(), 28);
  ASSERT_EQ(chunk1.size(), 28);
  ASSERT_EQ(chunk2.size(), 28);
  
  // chunk 0
  uint8_t const* ptr = reinterpret_cast<uint8_t const*>(chunk0.c_str());
  size_t chunkLength = fu::vst::parser::isChunkComplete(ptr, chunk0.size());
  ASSERT_EQ(chunkLength, 28);
  auto resultChunk = fu::vst::parser::readChunkHeaderVST1_1(ptr);
  ASSERT_EQ(resultChunk.header.chunkLength(), 28);
  ASSERT_EQ(resultChunk.header.messageID(), (1ULL << 56ULL) + 1ULL);
  ASSERT_EQ(resultChunk.header.messageLength(), 0x0C);
  ASSERT_TRUE(resultChunk.header.isFirst());
  ASSERT_EQ(resultChunk.header.index(), 0);
  ASSERT_EQ(resultChunk.header.numberOfChunks(), 3);
  ASSERT_EQ(resultChunk.body.size(), 4);
  ptr = reinterpret_cast<uint8_t const*>(resultChunk.body.data());
  uint32_t val = fu::basics::uintFromPersistentLittleEndian<uint32_t>(ptr);
  ASSERT_EQ(val, static_cast<uint32_t>(0x0d0c0b0a));
  
  // chunk 1
  ptr = reinterpret_cast<uint8_t const*>(chunk1.c_str());
  chunkLength = fu::vst::parser::isChunkComplete(ptr, chunk1.size());
  ASSERT_EQ(chunkLength, 28);
  resultChunk = fu::vst::parser::readChunkHeaderVST1_1(ptr);
  ASSERT_EQ(resultChunk.header.chunkLength(), 28);
  ASSERT_EQ(resultChunk.header.messageID(), (1ULL << 56ULL) + 1ULL);
  ASSERT_EQ(resultChunk.header.messageLength(), 0x0C);
  ASSERT_TRUE(!resultChunk.header.isFirst());
  ASSERT_EQ(resultChunk.header.index(), 1);
  ASSERT_EQ(resultChunk.body.size(), 4);
  ptr = reinterpret_cast<uint8_t const*>(resultChunk.body.data());
  val = fu::basics::uintFromPersistentLittleEndian<uint32_t>(ptr);
  ASSERT_EQ(val, static_cast<uint32_t>(0x0d0c0b0a));
  
  // chunk 2
  ptr = reinterpret_cast<uint8_t const*>(chunk2.c_str());
  chunkLength = fu::vst::parser::isChunkComplete(ptr, chunk2.size());
  ASSERT_EQ(chunkLength, 28);
  resultChunk = fu::vst::parser::readChunkHeaderVST1_1(ptr);
  ASSERT_EQ(resultChunk.header.chunkLength(), 28);
  ASSERT_EQ(resultChunk.header.messageID(), (1ULL << 56ULL) + 1ULL);
  ASSERT_EQ(resultChunk.header.messageLength(), 0x0C);
  ASSERT_TRUE(!resultChunk.header.isFirst());
  ASSERT_EQ(resultChunk.header.index(), 2);
  ASSERT_EQ(resultChunk.body.size(), 4);
  ptr = reinterpret_cast<uint8_t const*>(resultChunk.body.data());
  val = fu::basics::uintFromPersistentLittleEndian<uint32_t>(ptr);
  ASSERT_EQ(val, static_cast<uint32_t>(0x0d0c0b0a));
}
