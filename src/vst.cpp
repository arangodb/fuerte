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

#include <fuerte/helper.h>
#include <fuerte/types.h>
#include <sstream>
#include "portable_endian.h"
#include "vst.h"

#include <velocypack/Validator.h>
#include <velocypack/HexDump.h>
#include <velocypack/velocypack-aliases.h>

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {

using VValidator = ::arangodb::velocypack::Validator;
///////////////////////////////////////////////////////////////////////////////////
// sending vst
///////////////////////////////////////////////////////////////////////////////////

// ### not exported ###############################################################
// sending vst
static std::string chunkHeaderToString(ChunkHeader const& header){
  std::stringstream ss;
  ss << "### ChunkHeader ###"
     << "\nchunk length:         " << header.chunkLength()
     << "\nnumber of chunks:     " << header.numberOfChunks()
     << "\nmessage id:           " << header.messageID()
     << "\nis first:             " << header.isFirst()
     << "\nindex:                " << header.index()
     << "\ntotal message length: " << header.messageLength()
     << std::endl;
  return ss.str();
}

// writeHeaderToVST1_0 write the chunk to the given buffer in VST 1.0 format.
// The length of the buffer is returned.
size_t ChunkHeader::writeHeaderToVST1_0(VBuffer& buffer) const {
  size_t hdrLength;
  uint8_t hdr[maxChunkHeaderSize];
  if (isFirst() && numberOfChunks() > 1) {
		// Use extended header
    hdrLength = maxChunkHeaderSize;
    *(reinterpret_cast<uint64_t*>(hdr+16)) = htole64(_messageLength); // total message length
	} else {
		// Use minimal header
    hdrLength = minChunkHeaderSize;
	}
  auto dataSize = boost::asio::buffer_size(_data);
  *(reinterpret_cast<uint32_t*>(hdr+0)) = htole32(hdrLength + dataSize); // chunk length (header+data)
  *(reinterpret_cast<uint32_t*>(hdr+4)) = htole32(_chunkX);              // chunkX
  *(reinterpret_cast<uint64_t*>(hdr+8)) = htole64(_messageID);           // messageID

  // Now add hdr to buffer 
  buffer.append(hdr, hdrLength);
  return hdrLength;
}

// writeHeaderToVST1_1 write the chunk to the given buffer in VST 1.1 format.
// The length of the buffer is returned.
size_t ChunkHeader::writeHeaderToVST1_1(VBuffer& buffer) const {
  size_t hdrLength = maxChunkHeaderSize;
  uint8_t hdr[maxChunkHeaderSize];
  auto dataSize = boost::asio::buffer_size(_data);
  *(reinterpret_cast<uint32_t*>(hdr+0)) = htole32(hdrLength + dataSize); // chunk length (header+data)
  *(reinterpret_cast<uint32_t*>(hdr+4)) = htole32(_chunkX);              // chunkX
  *(reinterpret_cast<uint64_t*>(hdr+8)) = htole64(_messageID);           // messageID
  *(reinterpret_cast<uint64_t*>(hdr+16)) = htole64(_messageLength);      // total message length

  // Now add hdr to buffer 
  buffer.append(hdr, hdrLength);
  return hdrLength;
}

// buildChunks builds a list of chunks that are ready to be send to the server.
// The resulting set of chunks are added to the given result vector.
void buildChunks(uint64_t messageID, uint32_t maxChunkSize, std::vector<VSlice> const& messageParts, std::vector<ChunkHeader>& result) {
  // Check arguments 
  assert(maxChunkSize > maxChunkHeaderSize);

  // calculate total message length
  uint64_t messageLength = 0;
  for (auto it = std::begin(messageParts); it!=std::end(messageParts); ++it) {
    auto byteSize = it->byteSize();
    messageLength += byteSize;
  }

  // Build the chunks 
  result.clear();
  auto minChunkCount = messageLength / maxChunkSize;
  auto maxDataLength = maxChunkSize - maxChunkHeaderSize;
  uint32_t chunkIndex = 0;
  for (auto it = std::begin(messageParts); it!=std::end(messageParts); ++it) {
    size_t offset = 0;
    auto remaining = it->byteSize();
    while (remaining > 0) {
      auto dataLength = remaining;
      if (dataLength > maxDataLength) {
        dataLength = maxDataLength;
      }
      uint32_t chunkX = chunkIndex << 1;
      ChunkHeader chunk;
      chunk._chunkX = chunkX;
      chunk._messageID = messageID;
      chunk._messageLength = messageLength;
      chunk._data = boost::asio::const_buffer(it->start() + offset, dataLength);
      result.push_back(chunk);
      remaining -= dataLength;
      offset += dataLength;
      chunkIndex++;
    }
  }
  // Set chunkX of first chunk
	if (result.size() == 1) {
		result[0]._chunkX = 3;
	} else {
		result[0]._chunkX = (result.size() << 1) + 1;
	}
}

// section - VstMessageHeader

// createVstMessageHeader creates a slice containing a VST message header (array).
static std::string createVstMessageHeader(MessageHeader const& header)
{
  static std::string const message = " for message not set";
  VBuffer buffer;
  VBuilder builder(buffer);

  assert(builder.isClosed());
  builder.openArray();

  // 0 - version
  builder.add(VValue(header.version.get()));

  // 1 - type
  builder.add(VValue(static_cast<int>(header.type.get())));
  FUERTE_LOG_DEBUG << "MessageHeader.type=" << static_cast<int>(header.type.get()) << std::endl;
  switch (header.type.get()){
    case MessageType::Authentication:
      {
        // 2 - encryption
        auto encryption = header.encryption.get();
        builder.add(VValue(encryption));
        if (encryption == "plain") {
          // 3 - user 
          builder.add(VValue(header.user.get()));
          // 4 - password
          builder.add(VValue(header.password.get()));
        } else if (encryption == "jwt") {
          // 3 - token 
          builder.add(VValue(header.token.get()));
        } else {
          throw std::invalid_argument("unknown encryption: " + encryption);
        }
      }
      break;

    case MessageType::Request:
      // 2 - database
      if(!header.database){ throw std::runtime_error("database" + message); }
      builder.add(VValue(header.database.get()));
      FUERTE_LOG_DEBUG << "MessageHeader.database=" << header.database.get() << std::endl;

      // 3 - RestVerb
      if(!header.restVerb){ throw std::runtime_error("rest verb" + message); }
      builder.add(VValue(static_cast<int>(header.restVerb.get())));
      FUERTE_LOG_DEBUG << "MessageHeader.restVerb=" << static_cast<int>(header.restVerb.get()) << std::endl;

      // 4 - path
      if(!header.path){ throw std::runtime_error("path" + message); }
      builder.add(VValue(header.path.get()));
      FUERTE_LOG_DEBUG << "MessageHeader.path=" << header.path.get() << std::endl;

      // 5 - parameters - not optional in current server
      builder.openObject();
      if (header.parameters) {
        for (auto const& item : header.parameters.get()) {
          builder.add(item.first,VValue(item.second));
        }
      }
      builder.close();

      // 6 - meta
      builder.openObject();
      if (header.meta){
        for(auto const& item : header.meta.get()){
          builder.add(item.first,VValue(item.second));
        }
      }
      builder.close();

      break;

    case MessageType::Response:
      if(!header.responseCode){ throw std::runtime_error("response code" + message); }
      builder.add(VValue(header.responseCode.get()));                // 2
      break;
    default:
      break;
  }
  builder.close();

  return buffer.toString();
}

// ################################################################################

// prepareForNetwork prepares the internal structures for writing the request 
// to the network.
void RequestItem::prepareForNetwork(VSTVersion vstVersion) {
  // setting defaults
  _request->header.version = 1; // TODO vstVersionID;
  if(!_request->header.database){
    _request->header.database = "_system";
  }

  // Create the message header 
  _msgHdr = createVstMessageHeader(_request->header);

  // Split message into chunks
  std::vector<VSlice> slices(_request->slices());
  // Add message header slice to the front 
  slices.insert(slices.begin(), VSlice(_msgHdr.data()));
  std::vector<ChunkHeader> chunks;
  buildChunks(_messageID, defaultMaxChunkSize, slices, chunks);

  // Prepare request (write) buffers 
  _requestChunkBuffer.reserve(chunks.size() * maxChunkHeaderSize); // Reserve, so we don't have to re-allocate memory
  for (auto it = std::begin(chunks); it!=std::end(chunks); ++it) {
    auto chunkOffset = _requestChunkBuffer.byteSize();
    size_t chunkHdrLen;
    switch (vstVersion) {
      case VST1_0:
        chunkHdrLen = it->writeHeaderToVST1_0(_requestChunkBuffer);
        break;
      case VST1_1:
        chunkHdrLen = it->writeHeaderToVST1_1(_requestChunkBuffer);
        break;
      default:
        throw std::logic_error("Unknown VST version");
    }
    // Add chunk buffer 
    _requestBuffers.push_back(boost::asio::const_buffer(_requestChunkBuffer.data()+chunkOffset, chunkHdrLen));
    // Add chunk data buffer 
    _requestBuffers.push_back(it->_data);
  }
}

///////////////////////////////////////////////////////////////////////////////////
// receiving vst
///////////////////////////////////////////////////////////////////////////////////

// isChunkComplete returns the length of the chunk that starts at the given begin 
// if the entire chunk is available.
// Otherwise 0 is returned.
std::size_t isChunkComplete(uint8_t const * const begin, std::size_t const lengthAvailable) {
  if (lengthAvailable < sizeof(uint32_t)) { // there is not enought to read the length of
    return 0;
  }
  // read chunk length
  uint32_t lengthThisChunk = le32toh(*reinterpret_cast<const uint32_t*>(begin));
  if (lengthAvailable < lengthThisChunk) {
    FUERTE_LOG_VSTCHUNKTRACE << "\nchunk incomplete: " << lengthAvailable << "/" << lengthThisChunk << "(available/len)" << std::endl;
    return 0;
  }
  FUERTE_LOG_VSTCHUNKTRACE << "\nchunk complete: " << lengthThisChunk << " bytes" << std::endl;
  return lengthThisChunk;
}

// readChunkHeaderVST1_0 reads a chunk header in VST1.0 format.
ChunkHeader readChunkHeaderVST1_0(uint8_t const * const bufferBegin) {
  ChunkHeader header;

  auto hdr = bufferBegin;
  header._chunkLength = le32toh(*reinterpret_cast<const uint32_t*>(hdr+0));
  header._chunkX = le32toh(*reinterpret_cast<const uint32_t*>(hdr+4));
  header._messageID = le64toh(*reinterpret_cast<const uint64_t*>(hdr+8));
  size_t hdrLen = minChunkHeaderSize;

	if ((1 == (header._chunkX & 0x1)) && ((header._chunkX >> 1) > 1)) {
		// First chunk, numberOfChunks>1 -> read messageLength
		header._messageLength = le64toh(*reinterpret_cast<const uint64_t*>(hdr+16));
    hdrLen = maxChunkHeaderSize;
	}

  size_t contentLength = header._chunkLength - hdrLen;
  header._data = boost::asio::const_buffer(hdr+hdrLen, contentLength);
  FUERTE_LOG_VSTCHUNKTRACE << "readChunkHeaderVST1_0: got " << contentLength << " data bytes after " << hdrLen << " header bytes" << std::endl;

  return header;
}

// readChunkHeaderVST1_1 reads a chunk header in VST1.1 format.
ChunkHeader readChunkHeaderVST1_1(uint8_t const * const bufferBegin) {
  ChunkHeader header;

  auto hdr = bufferBegin;
  header._chunkLength = le32toh(*reinterpret_cast<const uint32_t*>(hdr+0));
  header._chunkX = le32toh(*reinterpret_cast<const uint32_t*>(hdr+4));
  header._messageID = le64toh(*reinterpret_cast<const uint64_t*>(hdr+8));
  header._messageLength = le64toh(*reinterpret_cast<const uint64_t*>(hdr+16));
  size_t contentLength = header._chunkLength - maxChunkHeaderSize;
  header._data = boost::asio::const_buffer(hdr+maxChunkHeaderSize, contentLength);
  FUERTE_LOG_VSTCHUNKTRACE << "readChunkHeaderVST1_1: got " << contentLength << " data bytes after " << std::endl;

  return header;
}

MessageHeader messageHeaderFromSlice(int vstVersionID, VSlice const& headerSlice){
  assert(headerSlice.isArray());
  MessageHeader header;
  header.byteSize = headerSlice.byteSize(); //for debugging

  if(vstVersionID == 1) {
    header.contentType(ContentType::VPack);
  } else {
    // found in params
    header.contentType(ContentType::Unset);
  }

  header.version = headerSlice.at(0).getNumber<int>();                              //version
  header.type = static_cast<MessageType>(headerSlice.at(1).getNumber<int>());       //type
  switch (header.type.get()){
    case MessageType::Authentication:
      //header.encryption = headerSlice.at(6);                                      //encryption (plain) should be 2
      header.user = headerSlice.at(2).copyString();                                 //user
      header.password = headerSlice.at(3).copyString();                             //password
      break;

    case MessageType::Request:
      header.database = headerSlice.at(2).copyString();                             // databse
      header.restVerb = static_cast<RestVerb>(headerSlice.at(3).getInt());          // rest verb
      header.path = headerSlice.at(4).copyString();                                 // request (path)
      header.parameters = sliceToStringMap(headerSlice.at(5));                      // query params
      header.meta = sliceToStringMap(headerSlice.at(6));                            // meta
      break;

    //resoponse should get content type
    case MessageType::Response:
      header.responseCode = headerSlice.at(2).getUInt(); // TODO fix me
      header.contentType(ContentType::VPack);
      if (headerSlice.length() >= 4) {
        header.meta = sliceToStringMap(headerSlice.at(3));                          // meta
      }
      break;
    default:
      break;
  }

  return header;
};

MessageHeader validateAndExtractMessageHeader(int const& vstVersionID, uint8_t const * const vpStart, std::size_t length, std::size_t& headerLength){
  using VValidator = ::arangodb::velocypack::Validator;
  // there must be at least one velocypack for the header
  VValidator validator;
  bool isSubPart = true;

  VSlice slice;
  try {
    // isSubPart allows the slice to be shorter than the checked buffer.
    validator.validate(vpStart, length , isSubPart);
    FUERTE_LOG_VSTTRACE << "validation done" << std::endl;
  } catch (std::exception const& e) {
    FUERTE_LOG_VSTTRACE << "len: " << length << std::string(reinterpret_cast<char const*>(vpStart), length);
    throw std::runtime_error(std::string("error during validation of incoming VPack (HEADER): ") + e.what());
  }
  slice = VSlice(vpStart);
  headerLength = slice.byteSize();

  return messageHeaderFromSlice(vstVersionID, slice);
}

std::size_t validateAndCount(uint8_t const * const vpStart, std::size_t length){
  // start points to the begin of a velocypack
  uint8_t const * cursor = vpStart;
  // there must be at least one velocypack for the header
  VValidator validator;
  std::size_t numPayloads = 0; // fist item is the header
  bool isSubPart = true;

  FUERTE_LOG_VSTTRACE << "buffer length to validate: " << length << std::endl;


  FUERTE_LOG_VSTTRACE << "sliceSizes: ";
  VSlice slice;
  while (length) {
    try {
      // isSubPart allows the slice to be shorter than the checked buffer.
      validator.validate(cursor, length , isSubPart);
      slice = VSlice(cursor);
      auto sliceSize = slice.byteSize();
      if(length < sliceSize){
        throw std::length_error("slice is longer than buffer");
      }
      length -= sliceSize;
      cursor += sliceSize;
      numPayloads++;
      FUERTE_LOG_VSTTRACE << sliceSize << " ";
    } catch (std::exception const& e) {
      FUERTE_LOG_VSTTRACE << "len: " << length << VPackHexDump(slice);
      FUERTE_LOG_VSTTRACE << "len: " << length << std::string(reinterpret_cast<char const*>(cursor), length);
      throw std::runtime_error(std::string("error during validation of incoming VPack (body)") + e.what());
    }
  }
  FUERTE_LOG_VSTTRACE << std::endl;
  return numPayloads;
}

// add the given chunk to the list of response chunks.
void RequestItem::addChunk(ChunkHeader& chunk) {
  // Copy _data to response buffer 
  auto contentStart = boost::asio::buffer_cast<const uint8_t*>(chunk._data);
  chunk._responseContentLength = boost::asio::buffer_size(chunk._data);
  FUERTE_LOG_VSTCHUNKTRACE << "RequestItem::addChunk: adding " << chunk._responseContentLength << " bytes to buffer" << std::endl;
  chunk._responseChunkContentOffset = _responseChunkContent.byteSize();
  _responseChunkContent.append(contentStart, chunk._responseContentLength);
  // Release _data in chunk 
  chunk._data = boost::asio::const_buffer();
  // Add chunk to list 
  _responseChunks.push_back(chunk);
  // Gather number of chunk info 
  if (chunk.isFirst()) {
    _responseNumberOfChunks = chunk.numberOfChunks();
    FUERTE_LOG_VSTCHUNKTRACE << "RequestItem::addChunk: set #chunks to " << _responseNumberOfChunks << std::endl;
  }
}

static bool chunkByIndex (const ChunkHeader& a, const ChunkHeader& b) { return (a.index() < b.index()); }

// try to assembly the received chunks into a buffer.
// returns NULL if not all chunks are available.
std::unique_ptr<VBuffer> RequestItem::assemble() {
  if (_responseNumberOfChunks == 0) {
		// We don't have the first chunk yet
    FUERTE_LOG_VSTCHUNKTRACE << "RequestItem::assemble: don't have first chunk" << std::endl;
		return nullptr;
	}
	if (_responseChunks.size() < _responseNumberOfChunks) {
		// Not all chunks have arrived yet
    FUERTE_LOG_VSTCHUNKTRACE << "RequestItem::assemble: not all chunks have arrived" << std::endl;
		return nullptr;
	}

  // We now have all chunks. Sort them by index.
  FUERTE_LOG_VSTCHUNKTRACE << "RequestItem::assemble: sort chunks" << std::endl;
  std::sort (_responseChunks.begin(), _responseChunks.end(), chunkByIndex);

  // Combine chunk content 
  FUERTE_LOG_VSTCHUNKTRACE << "RequestItem::assemble: build response buffer" << std::endl;
  auto buffer = std::unique_ptr<VBuffer>(new VBuffer());
  for (auto it = std::begin(_responseChunks); it!=std::end(_responseChunks); ++it) {
    buffer->append(_responseChunkContent.data() + it->_responseChunkContentOffset, it->_responseContentLength);
  }

  return buffer;
}

}}}}
