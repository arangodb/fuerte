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

#include <string>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <mutex>

#include <boost/asio/buffer.hpp>

#include <fuerte/message.h>
#include <fuerte/types.h>
#include <fuerte/FuerteLogger.h>

#include "CallOnceRequestCallback.h"

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {

class IncompleteMessage;
using MessageID = uint64_t;

static size_t const bufferLength = 4096UL;
//static size_t const chunkMaxBytes = 1000UL;
static size_t const	minChunkHeaderSize = 16;
static size_t const	maxChunkHeaderSize = 24;
static size_t const defaultMaxChunkSize = 30000;

static const char *vstHeader1_0 = "VST/1.0\r\n\r\n";
static const char *vstHeader1_1 = "VST/1.1\r\n\r\n";

/////////////////////////////////////////////////////////////////////////////////////
// DataStructures
/////////////////////////////////////////////////////////////////////////////////////

// Velocystream Chunk Header
struct ChunkHeader {
  // data used in the specification
  uint32_t _chunkLength;           // length of this chunk includig chunkHeader 
  uint32_t _chunkX;                // number of chunks or chunk number
  uint64_t _messageID;             // messageid
  uint64_t _messageLength;         // length of total payload

  // internal data
  boost::asio::const_buffer _data;    // Reference to data that belongs to this chunk.
  size_t _responseChunkContentOffset; // Offset of start of content of this chunk in RequestItem._responseChunkContent.
  size_t _responseContentLength;      // Content length of this chunk (only used during read operations).

  // Return length of this chunk (in host byte order)
  inline uint32_t chunkLength() const { return _chunkLength; }
  // Return message ID of this chunk (in host byte order)
  inline uint64_t messageID() const { return _messageID; }
  // Return total message length (in host byte order)
  inline uint64_t messageLength() const { return _messageLength; }
  // isFirst returns true when the "first chunk" flag has been set.
  inline bool isFirst() const { return ((_chunkX & 0x01) == 1); }
  // index returns the index of this chunk in the message.
  inline uint32_t index() const { 
    if ((_chunkX & 0x01) == 1) {
      return 0;
    }
    return _chunkX >> 1;
  }
  // numberOfChunks return the number of chunks that make up the entire message.
  // This function is only valid for first chunks.
  inline uint32_t numberOfChunks() const { 
    if ((_chunkX & 0x01) == 1) {
      return _chunkX >> 1;
    }
    return 0; // Not known
  }

  // writeHeaderToVST1_0 write the chunk to the given buffer in VST 1.0 format.
  // The length of the buffer is returned.
  size_t writeHeaderToVST1_0(VBuffer& buffer) const;

  // writeHeaderToVST1_1 write the chunk to the given buffer in VST 1.1 format.
  // The length of the buffer is returned.
  size_t writeHeaderToVST1_1(VBuffer& buffer) const;
};

// buildChunks builds a list of chunks that are ready to be send to the server.
// The resulting set of chunks are added to the given result vector.
void buildChunks(uint64_t messageID, uint32_t maxChunkSize, std::vector<VSlice> const& messageParts, std::vector<ChunkHeader>& result);

// chunkHeaderLength returns the length of a VST chunk header for given arguments.
inline std::size_t chunkHeaderLength(VSTVersion vstVersion, bool isFirst, bool isSingle) {
  switch (vstVersion) {
    case VST1_0:
      if (isFirst && !isSingle) {
        return maxChunkHeaderSize;
      }
      return minChunkHeaderSize;
    case VST1_1:
      return maxChunkHeaderSize;
    default:
      throw std::logic_error("Unknown VST version");
  }
}

// Item that represents a Request in flight
struct RequestItem {
  std::unique_ptr<Request> _request;  // Reference to the request we're processing 
  impl::CallOnceRequestCallback _callback;           // Callback for when request is done (in error or succeeded)
  MessageID _messageID;               // ID of this message
  // Request variables
  std::string _msgHdr;                // VST message header
  VBuffer _requestChunkBuffer;        // Buffer used to hold chunk headers
  std::vector<boost::asio::const_buffer> _requestBuffers; // Buffers the will be send to the socket.
  // Response variables
  std::vector<ChunkHeader> _responseChunks; // List of chunks that have been received.
  VBuffer _responseChunkContent;      // Buffer containing content of received chunks. (this is not in sorted order!)
  size_t _responseNumberOfChunks;     // The number of chunks we're expecting (0==not know yet).

  inline MessageID messageID() { return _messageID; }
  inline void invokeOnError(Error e, std::unique_ptr<Request> req, std::unique_ptr<Response> res) { 
    _callback.invoke(e, std::move(req), std::move(res));
  }

  // prepareForNetwork prepares the internal structures for writing the request 
  // to the network.
  void prepareForNetwork(VSTVersion);

  // add the given chunk to the list of response chunks.
  void addChunk(ChunkHeader&);
  // try to assembly the received chunks into a response.
  // returns NULL if not all chunks are available.
  std::unique_ptr<VBuffer> assemble();

  // Flush all memory needed for sending this request.
  inline void resetSendData() {
    _msgHdr.clear();
    _requestBuffers.clear();
    _requestChunkBuffer.clear();
  }
};

/////////////////////////////////////////////////////////////////////////////////////
// receive vst
/////////////////////////////////////////////////////////////////////////////////////

// isChunkComplete returns the length of the chunk that starts at the given begin 
// if the entire chunk is available.
// Otherwise 0 is returned.
std::size_t isChunkComplete(uint8_t const * const begin, std::size_t const length);

// readChunkHeaderVST1_0 reads a chunk header in VST1.0 format.
ChunkHeader readChunkHeaderVST1_0(uint8_t const * const bufferBegin);

// readChunkHeaderVST1_1 reads a chunk header in VST1.1 format.
ChunkHeader readChunkHeaderVST1_1(uint8_t const * const bufferBegin);

// creates a MessageHeader form a given slice
MessageHeader messageHeaderFromSlice(VSlice const& headerSlice);
// validates if a data range contains a slice and converts it
// to a message Hader and returns size occupied by the sloce via reference
MessageHeader validateAndExtractMessageHeader(int const& vstVersionID, uint8_t const * const vpStart, std::size_t length, std::size_t& headerLength);\

//Validates if payload consitsts of valid velocypack slices
std::size_t validateAndCount(uint8_t const* vpHeaderStart, std::size_t len);

}}}}
#endif
