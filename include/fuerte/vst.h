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
#pragma once
#ifndef ARANGO_CXX_DRIVER_VST
#define ARANGO_CXX_DRIVER_VST

#include "types.h"

#include <string>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include <fuerte/message.h>
#include "FuerteLogger.h"

namespace std {
  class mutex;
}

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {

class IncompleteMessage;
using MessageID = uint64_t;

static size_t const bufferLength = 4096UL;
static size_t const chunkMaxBytes = 1000UL;

/////////////////////////////////////////////////////////////////////////////////////
// DataStructures
/////////////////////////////////////////////////////////////////////////////////////

// Velocystream Chunk Header
struct ChunkHeader {
  // data used in the specification
  uint32_t _chunkLength;           // length of this chunk includig chunkHeader
  uint32_t _chunk;                 // number of chunks or chunk number
  uint64_t _messageID;             // messageid
  uint64_t _totalMessageLength;    // length of total unencrypeted payload + vstMessageHeader
                                   // but without chunk headers

  // additional data that is not in the protocl
  std::size_t _chunkHeaderLength;  // lenght of vstChunkHeader
  uint32_t _chunkPayloadLength;    // length of payload for this chunk
  uint32_t _numberOfChunks;
  bool _isSingle;                  // is a single chunk?
  bool _isFirst;                   // is first or followup chunk -- encoded in chunk

  //update chunk len in structure and in an already existing buffer
  uint32_t updateChunkPayload(uint8_t* headerStartInBuffer, uint32_t payloadLength){
    _chunkPayloadLength = payloadLength;
    _chunkLength = _chunkHeaderLength + _chunkPayloadLength;
    //update chunk len in buffer
    std::memcpy(headerStartInBuffer, &_chunkLength, sizeof(_chunkLength)); //target, source, length
    FUERTE_LOG_VSTTRACE << "chunk length set to: " << _chunkLength
                        << " = "
                        << _chunkHeaderLength << " + " << _chunkPayloadLength
                        << std::endl;
    return _chunkLength;
  }

  uint32_t updateTotalPayload(uint8_t* headerStartInBuffer, uint64_t messageLength){
    _totalMessageLength = messageLength;
    auto pos = headerStartInBuffer + sizeof(_chunkLength) + sizeof(_chunk) + sizeof(_messageID);
    std::memcpy(pos, &_totalMessageLength, sizeof(_totalMessageLength)); //target, source, length

    FUERTE_LOG_DEBUG << "totalMessageLength set to: " << _totalMessageLength
                     << std::endl;
    return _totalMessageLength;
  }
};

inline constexpr std::size_t chunkHeaderLength(int version, bool isFirst, bool isSingle){
  // until there is the next version we should use c++14 :P
  return (version == 1) ?
    sizeof(ChunkHeader::_chunkLength) + sizeof(ChunkHeader::_chunk) +
    sizeof(ChunkHeader::_messageID) + (isFirst && !isSingle ? sizeof(ChunkHeader::_totalMessageLength) : 0)
    :
    sizeof(ChunkHeader::_chunkLength) + sizeof(ChunkHeader::_chunk) + sizeof(ChunkHeader::_messageID)
    ;
}

// Item that represents a Request in flight
struct RequestItem {
  std::unique_ptr<Request> _request;
  OnErrorCallback _onError;
  OnSuccessCallback _onSuccess;
  MessageID _messageId;
  std::shared_ptr<VBuffer> _requestBuffer;
  VBuffer _responseBuffer;
  uint32_t _responseLength;    // length of complete message in bytes
  std::size_t _responseChunks; // number of chunks in response
  std::size_t _responseChunk;  // nuber of current chunk
};

/////////////////////////////////////////////////////////////////////////////////////
// send vst
/////////////////////////////////////////////////////////////////////////////////////

// creates a buffer in a shared pointer containg the message read to be send
// out as vst (ChunkHeader, Header, Payload)
std::shared_ptr<VBuffer> toNetwork(Request&);

/////////////////////////////////////////////////////////////////////////////////////
// receive vst
/////////////////////////////////////////////////////////////////////////////////////

// find out if data between begin and end assembles a complete
// Velocystream chunk. If so return offset to chunk end or 0 if chunk is not complete
std::size_t isChunkComplete(uint8_t const * const begin, std::size_t const length);

// If there is a complete VstChunk you can use this function to read the header
// a version 1.0 Header into a data structure
ChunkHeader readChunkHeaderV1_0(uint8_t const * const bufferBegin);

// creates a MessageHeader form a given slice
MessageHeader messageHeaderFromSlice(VSlice const& headerSlice);
// validates if a data range contains a slice and converts it
// to a message Hader and returns size occupied by the sloce via reference
MessageHeader validateAndExtractMessageHeader(int const& vstVersionID, uint8_t const * const vpStart, std::size_t length, std::size_t& headerLength);\

//Validates if payload consitsts of valid velocypack slices
std::size_t validateAndCount(uint8_t const* vpHeaderStart, std::size_t len);

}}}}
#endif
