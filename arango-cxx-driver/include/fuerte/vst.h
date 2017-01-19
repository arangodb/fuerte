#pragma once
#include "types.h"

#include <string>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include <fuerte/message.h>
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
  uint64_t _totalMessageLength;    // length of total unencrypeted payload +vstMessageHeader

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
    return _chunkLength;
  }

  uint32_t updateTotalPayload(uint8_t* headerStartInBuffer, uint64_t messageLength){
    _totalMessageLength = messageLength;
    auto pos = headerStartInBuffer + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t);
    std::memcpy(pos, &_totalMessageLength, sizeof(_totalMessageLength)); //target, source, length
    return _totalMessageLength;
  }
};

inline constexpr std::size_t chunkHeaderLength(int version, bool isFirst){
  // until there is the next version we should use c++14 :P
  return (version == 1) ?
    // chunkLength uint32 , chunkX uint32 , id uint64 , messageLength unit64
    sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + (isFirst ? sizeof(uint64_t) : 0)
    :
    // chunkLength uint32 , chunkX uint32 , id uint64 , messageLength unit64
    sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint64_t)
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

// for the next function VBuffer must contain a complete message
inline std::unique_ptr<Response> fromNetwork(VBuffer&){ throw std::logic_error("implement me!"); }

// creates a MessageHeader form a given slice
MessageHeader messageHeaderFromSlice(VSlice const& headerSlice);
// validates if a data range contains a slice and converts it
// to a message Hader and returns size occupied by the sloce via reference
MessageHeader validateAndExtractMessageHeader(uint8_t const * const vpStart, std::size_t length, std::size_t& headerLength);\

//Validates if payload consitsts of valid velocypack slices
std::size_t validateAndCount(uint8_t const* vpHeaderStart, std::size_t len);

}}}}

//// this function will be called when we send multiple compressed
//// or uncompressed chunks
//inline void send_many(
//    std::vector<std::unique_ptr<basics::StringBuffer>>& resultVecRef,
//    uint64_t id, std::size_t maxChunkBytes,
//    std::unique_ptr<basics::StringBuffer> completeMessage,
//    std::size_t uncompressedCompleteMessageLength) {
//  uint64_t totalLen = completeMessage->length();
//  std::size_t offsetBegin = 0;
//  std::size_t offsetEnd = maxChunkBytes - chunkHeaderLength(true);
//  // maximum number of bytes for follow up chunks
//  std::size_t maxBytes = maxChunkBytes - chunkHeaderLength(false);
//
//  uint32_t numberOfChunks = 1;
//  {  // calcuate the number of chunks taht will be send
//    std::size_t bytesToSend = totalLen - maxChunkBytes +
//                              chunkHeaderLength(true);  // data for first chunk
//    while (bytesToSend >= maxBytes) {
//      bytesToSend -= maxBytes;
//      ++numberOfChunks;
//    }
//    if (bytesToSend) {
//      ++numberOfChunks;
//    }
//  }
//
//  // send fist
//  resultVecRef.push_back(
//      createChunkForNetworkMultiFirst(completeMessage->c_str(), offsetBegin,
//                                      offsetEnd, id, numberOfChunks, totalLen));
//
//  std::uint32_t chunkNumber = 0;
//  while (offsetEnd + maxBytes <= totalLen) {
//    // send middle
//    offsetBegin = offsetEnd;
//    offsetEnd += maxBytes;
//    chunkNumber++;
//    resultVecRef.push_back(createChunkForNetworkMultiFollow(
//        completeMessage->c_str(), offsetBegin, offsetEnd, id, chunkNumber));
//  }
//
//  if (offsetEnd < totalLen) {
//    resultVecRef.push_back(createChunkForNetworkMultiFollow(
//        completeMessage->c_str(), offsetEnd, totalLen, id, ++chunkNumber));
//  }
//
//  return;
//}
//
//// this function will be called by client code
//inline std::vector<std::unique_ptr<basics::StringBuffer>> createChunkForNetwork(
//    std::vector<VPackSlice> const& slices, uint64_t id,
//    std::size_t maxChunkBytes, bool compress = false) {
//  /// variables used in this function
//  std::size_t uncompressedPayloadLength = 0;
//  // worst case len in case of compression
//  std::size_t preliminaryPayloadLength = 0;
//  // std::size_t compressedPayloadLength = 0;
//  std::size_t payloadLength = 0;  // compressed or uncompressed
//
//  std::vector<std::unique_ptr<basics::StringBuffer>> rv;
//
//  // find out the uncompressed payload length
//  for (auto const& slice : slices) {
//    uncompressedPayloadLength += slice.byteSize();
//  }
//
//  if (compress) {
//    // use some function to calculate the worst case lenght
//    preliminaryPayloadLength = uncompressedPayloadLength;
//  } else {
//    payloadLength = uncompressedPayloadLength;
//  }
//
//  if (!compress &&
//      uncompressedPayloadLength < maxChunkBytes - chunkHeaderLength(false)) {
//    // one chunk uncompressed
//    rv.push_back(createChunkForNetworkSingle(slices, id));
//    return rv;
//  } else if (compress &&
//             preliminaryPayloadLength <
//                 maxChunkBytes - chunkHeaderLength(false)) {
//    throw std::logic_error("no implemented");
//    // one chunk compressed
//  } else {
//    //// here we enter the domain of multichunck
//    LOG_TOPIC(DEBUG, Logger::COMMUNICATION)
//        << "VppCommTask: sending multichunk message";
//
//    // test if we have smaller slices that fit into chunks when there is
//    // no compression - optimization
//
//    LOG_TOPIC(DEBUG, Logger::COMMUNICATION)
//        << "VppCommTask: there are slices that do not fit into a single "
//           "totalMessageLength or compression is enabled";
//    // we have big slices that do not fit into single chunks
//    // now we will build one big buffer ans split it into pieces
//
//    // reseve buffer
//    auto vppPayload = std::make_unique<basics::StringBuffer>(
//        TRI_UNKNOWN_MEM_ZONE, uncompressedPayloadLength, false);
//
//    // fill buffer
//    for (auto const& slice : slices) {
//      vppPayload->appendText(slice.startAs<char>(), slice.byteSize());
//    }
//
//    if (compress) {
//      // compress uncompressedVppPayload -> vppPayload
//      auto uncommpressedVppPayload = std::move(vppPayload);
//      vppPayload = std::make_unique<basics::StringBuffer>(
//          TRI_UNKNOWN_MEM_ZONE, preliminaryPayloadLength, false);
//      // do compression
//      throw std::logic_error("no implemented");
//      // payloadLength = compressedPayloadLength;
//    }
//
//    // create chunks
//    (void)payloadLength;
//    send_many(rv, id, maxChunkBytes, std::move(vppPayload),
//              uncompressedPayloadLength);
//  }
//  return rv;
//}
//
//#endif
