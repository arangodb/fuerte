#pragma once
#include "common_types.h"

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

// Velocystream Header
struct Header {
  std::size_t headerLength;
  uint32_t chunkLength;
  uint32_t chunk;
  uint64_t messageID;
  uint64_t messageLength;
  bool isFirst;
  bool isSingle;
};

struct ReadBufferInfo {
	ReadBufferInfo()
		: currentChunkLength(0)
    , readBufferOffset(0)
    , cleanupLength(bufferLength - chunkMaxBytes - 1)
		{}

    uint32_t currentChunkLength; // size of chunk processed or 0 when expecting
                                 // new chunk
    size_t readBufferOffset;     // data up to this position has been processed
    std::size_t cleanupLength;   // length of data after that the read buffer
                                 // will be cleaned
  };

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

std::shared_ptr<VBuffer> toNetwork(Request const&);
/////////////////////////////////////////////////////////////////////////////////////
// send vst
/////////////////////////////////////////////////////////////////////////////////////

// find out if data between begin and end assembles a complete
// Velocystream chunk. If so return offset to chunk end or 0 if chunk is not complete
std::size_t isChunkComplete(uint8_t const * const begin, std::size_t const length);

// If there is a complete VstChunk you can use this function to read the header
// a version 1.0 Header into a datasructure
Header readHeaderV1_0(uint8_t const * const bufferBegin);

std::size_t validateAndCount(uint8_t const* vpHeaderStart, std::size_t len);

std::unique_ptr<Response> fromNetwork( NetBuffer const&
                                     , std::map<MessageID,std::shared_ptr<RequestItem>>& map
                                     , std::mutex& mapMutex
                                     , std::size_t& consumed
                                     , bool& complete
                                     );

















//template <typename T>
//std::size_t appendToBuffer(basics::StringBuffer* buffer, T& value) {
//  constexpr std::size_t len = sizeof(T);
//  char charArray[len];
//  char const* charPtr = charArray;
//  std::memcpy(&charArray, &value, len);
//  buffer->appendText(charPtr, len);
//  return len;
//}
//
//inline constexpr std::size_t chunkHeaderLength(bool firstOfMany) {
//  // chunkLength uint32 , chunkX uint32 , id uint64 , messageLength unit64
//  return sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) +
//         (firstOfMany ? sizeof(uint64_t) : 0);
//}
//
//// Send Message Created from Slices
//
//// working version of single chunk message creation
//inline std::unique_ptr<basics::StringBuffer> createChunkForNetworkDetail(
//    std::vector<VPackSlice> const& slices, bool isFirstChunk, uint32_t chunk,
//    uint64_t id, uint64_t totalMessageLength = 0) {
//  using basics::StringBuffer;
//  bool firstOfMany = false;
//
//  // if we have more than one chunk and the chunk is the first
//  // then we are sending the first in a series. if this condition
//  // is true we also send extra 8 bytes for the messageLength
//  // (length of all VPackData)
//  if (isFirstChunk && chunk > 1) {
//    firstOfMany = true;
//  }
//
//  // build chunkX -- see VelocyStream Documentaion
//  chunk <<= 1;
//  chunk |= isFirstChunk ? 0x1 : 0x0;
//
//  // get the lenght of VPack data
//  uint32_t dataLength = 0;
//  for (auto& slice : slices) {
//    // TODO: is a 32bit value sufficient for all Slices here?
//    dataLength += static_cast<uint32_t>(slice.byteSize());
//  }
//
//  // calculate length of current chunk
//  uint32_t chunkLength =
//      dataLength + static_cast<uint32_t>(chunkHeaderLength(firstOfMany));
//
//  auto buffer =
//      std::make_unique<StringBuffer>(TRI_UNKNOWN_MEM_ZONE, chunkLength, false);
//
//  appendToBuffer(buffer.get(), chunkLength);
//  appendToBuffer(buffer.get(), chunk);
//  appendToBuffer(buffer.get(), id);
//
//  if (firstOfMany) {
//    appendToBuffer(buffer.get(), totalMessageLength);
//  }
//
//  // append data in slices
//  for (auto const& slice : slices) {
//    buffer->appendText(slice.startAs<char>(), slice.byteSize());
//  }
//
//  return buffer;
//}
//
////  slices, isFirstChunk, chunk, id, totalMessageLength
//inline std::unique_ptr<basics::StringBuffer> createChunkForNetworkSingle(
//    std::vector<VPackSlice> const& slices, uint64_t id) {
//  return createChunkForNetworkDetail(slices, true, 1, id, 0 /*unused*/);
//}
//
//// This method does not respect the max chunksize instead it avoids copying
//// by moving slices into the createion functions - This is not acceptable for
//// big slices
////
//// inline std::unique_ptr<basics::StringBuffer> createChunkForNetworkMultiFirst(
////    std::vector<VPackSlice> const& slices, uint64_t id, uint32_t
////    numberOfChunks,
////    uint32_t totalMessageLength) {
////  return createChunkForNetworkDetail(slices, true, numberOfChunks, id,
////                                     totalMessageLength);
////}
////
//// inline std::unique_ptr<basics::StringBuffer>
//// createChunkForNetworkMultiFollow(
////    std::vector<VPackSlice> const& slices, uint64_t id, uint32_t chunkNumber,
////    uint32_t totalMessageLength) {
////  return createChunkForNetworkDetail(slices, false, chunkNumber, id, 0);
////}
//
//// helper functions for sending chunks when given a string buffer as input
//// working version of single chunk message creation
//inline std::unique_ptr<basics::StringBuffer> createChunkForNetworkDetail(
//    char const* data, std::size_t begin, std::size_t end, bool isFirstChunk,
//    uint32_t chunk, uint64_t id, uint64_t totalMessageLength = 0) {
//  using basics::StringBuffer;
//  bool firstOfMany = false;
//
//  // if we have more than one chunk and the chunk is the first
//  // then we are sending the first in a series. if this condition
//  // is true we also send extra 8 bytes for the messageLength
//  // (length of all VPackData)
//  if (isFirstChunk && chunk > 1) {
//    firstOfMany = true;
//  }
//
//  // build chunkX -- see VelocyStream Documentaion
//  chunk <<= 1;
//  chunk |= isFirstChunk ? 0x1 : 0x0;
//
//  // get the lenght of VPack data
//  uint32_t dataLength = static_cast<uint32_t>(end - begin);
//
//  // calculate length of current chunk
//  uint32_t chunkLength =
//      dataLength + static_cast<uint32_t>(chunkHeaderLength(firstOfMany));
//
//  auto buffer =
//      std::make_unique<StringBuffer>(TRI_UNKNOWN_MEM_ZONE, chunkLength, false);
//
//  appendToBuffer(buffer.get(), chunkLength);
//  appendToBuffer(buffer.get(), chunk);
//  appendToBuffer(buffer.get(), id);
//
//  if (firstOfMany) {
//    appendToBuffer(buffer.get(), totalMessageLength);
//  }
//
//  buffer->appendText(data + begin, dataLength);
//
//  return buffer;
//}
//
//inline std::unique_ptr<basics::StringBuffer> createChunkForNetworkMultiFirst(
//    char const* data, std::size_t begin, std::size_t end, uint64_t id,
//    uint32_t numberOfChunks, uint64_t totalMessageLength) {
//  return createChunkForNetworkDetail(data, begin, end, true, numberOfChunks, id,
//                                     totalMessageLength);
//}
//
//inline std::unique_ptr<basics::StringBuffer> createChunkForNetworkMultiFollow(
//    char const* data, std::size_t begin, std::size_t end, uint64_t id,
//    uint32_t chunkNumber) {
//  return createChunkForNetworkDetail(data, begin, end, false, chunkNumber, id,
//                                     0);
//}
//
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
//
//
//

}}}}
