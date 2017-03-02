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
#include <fuerte/vst.h>
#include <sstream>
#include <velocypack/Validator.h>

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
     << "\nchunk length:         " << header._chunkLength
     << "\nchunk header length:  " << header._chunkHeaderLength
     << "\nchunk payload length: " << header._chunkPayloadLength
     << "\nnumber of chunks:     " << header._numberOfChunks
     << "\nmessage id:           " << header._messageID
     << "\nis first:             " << header._isFirst
     << "\nis single:            " << header._isSingle
     << "\ntotal message length: " << header._chunkLength
     << "\nchunk:                " << header._chunk
     << std::endl;
  return ss.str();
}

// section - VstChunkHeader
static ChunkHeader createChunkHeader(int vstVersionID
                                    ,MessageID messageID
                                    ,std::size_t totalMessageLength
                                    ,std::size_t chunkPayloadLength
                                    ,std::size_t chunk
                                    ,bool isFirst){

  ChunkHeader header;
  header._chunkPayloadLength = chunkPayloadLength;
  header._numberOfChunks = chunk;
  header._isFirst = isFirst;
  header._chunk = chunk;
  header._chunk <<= 1;
  header._chunk |= header._isFirst ? 0x1 : 0x0;
  header._isSingle = isFirst && (header._numberOfChunks == 1);
  header._chunkHeaderLength  = chunkHeaderLength(vstVersionID, isFirst, header._isSingle);
  header._chunkLength = header._chunkHeaderLength + header._chunkPayloadLength;
  header._messageID = messageID;
  header._totalMessageLength = totalMessageLength; //total payload length
  return header;
}

// sometimes it is hard to know th payload length for a chunk in advance
static ChunkHeader createSingleChunkHeader(int vstVersionID
                                         ,MessageID messageID
                                         ,std::size_t chunkPayloadLength
                                         ){
  return createChunkHeader(vstVersionID, messageID, chunkPayloadLength
                          ,chunkPayloadLength, 1, true);
}

static ChunkHeader createFirstChunkHeader(int vstVersionID
                                         ,MessageID messageID
                                         ,std::size_t chunkPayloadLength
                                         ,std::size_t totoalMessageLength
                                         ,std::size_t numberOfChunks){
  return createChunkHeader(vstVersionID, messageID, chunkPayloadLength
                          ,totoalMessageLength, numberOfChunks, false);
}

static ChunkHeader createFollowUpChunkHeader(int vstVersionID
                                         ,MessageID messageID
                                         ,std::size_t chunkPayloadLength
                                         ,std::size_t totalMessageLength
                                         ,std::size_t numberOfChunks){
  return createChunkHeader(vstVersionID, messageID, chunkPayloadLength
                          ,totalMessageLength, numberOfChunks, false);
}


template <typename T>
std::size_t appendToBuffer(VBuffer& buffer, T &value) {
  //TODO -- fix endianess
  constexpr std::size_t len = sizeof(T);
  uint8_t charArray[len];
  uint8_t* charPtr = charArray;
  std::memcpy(&charArray, &value, len);
  buffer.append(charPtr, len);
  return len;
}

static std::size_t addVstChunkHeader(std::size_t vstVersionID
                                    ,VBuffer& buffer
                                    ,ChunkHeader const& header)
{
  appendToBuffer(buffer, header._chunkLength);            // 1 - length    - 4 byte
  appendToBuffer(buffer, header._chunk);                  // 2 - chunkX    - 4 byte
  appendToBuffer(buffer, header._messageID);              // 3 - messageid - 8 byte

  if ((header._isFirst && header._numberOfChunks > 1) || (vstVersionID > 1)) {
    appendToBuffer(buffer, header._totalMessageLength);   // 4 - lmessage length - 8 byte
  }

  return buffer.byteSize();
}

// section - VstMessageHeader
static void addVstMessageHeader(VBuilder& builder
                               ,MessageHeader const& header
                               )
{
  static std::string const message = " for message not set";
  auto startSize = builder.size();

  assert(builder.isClosed());
  builder.openArray();

  // 0 - version
  builder.add(VValue(header.version.get()));

  // 1 - type
  builder.add(VValue(static_cast<int>(header.type.get())));
  //FUERTE_LOG_DEBUG << "MessageHeader.type=" << static_cast<int>(header.type.get()) << std::endl;
  switch (header.type.get()){
    case MessageType::Authentication:
      //builder.add(VValue(header.encryption.get()));
      if(!header.user){ throw std::runtime_error("user" + message); }
      builder.add(VValue(header.user.get()));
      //FUERTE_LOG_DEBUG << "MessageHeader.user=" << header.user.get() << std::endl;
      if(!header.password){ throw std::runtime_error("password" + message); }
      builder.add(VValue(header.password.get()));
      //FUERTE_LOG_DEBUG << "MessageHeader.password=" << header.password.get() << std::endl;
      break;

    case MessageType::Request:
      // 2 - database
      if(!header.database){ throw std::runtime_error("database" + message); }
      builder.add(VValue(header.database.get()));
      //FUERTE_LOG_DEBUG << "MessageHeader.database=" << header.database.get() << std::endl;

      // 3 - RestVerb
      if(!header.restVerb){ throw std::runtime_error("rest verb" + message); }
      builder.add(VValue(static_cast<int>(header.restVerb.get())));
      //FUERTE_LOG_DEBUG << "MessageHeader.restVerb=" << static_cast<int>(header.restVerb.get()) << std::endl;

      // 4 - path
      if(!header.path){ throw std::runtime_error("path" + message); }
      builder.add(VValue(header.path.get()));
      //FUERTE_LOG_DEBUG << "MessageHeader.path=" << header.path.get() << std::endl;

      // 5 - parameters - not optional in current server
      builder.openObject();
      if (header.parameter){
        for(auto const& item : header.parameter.get()){
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
}

// ################################################################################

std::shared_ptr<VBuffer> toNetwork(Request& request){
  auto buffer = std::make_shared<VBuffer>();
  std::size_t const vstVersionID = 1;

  // setting defaults
  request.header.version = vstVersionID;
  if(!request.header.database){
    request.header.database = "_system";
  }

  // add chunk header
  auto vstChunkHeader = createSingleChunkHeader(vstVersionID, request.messageid, 0); //size is unfortunatly unknown
  auto chunkHeaderLength = addVstChunkHeader(std::size_t(1), *buffer, vstChunkHeader);
  FUERTE_LOG_VSTTRACE << "ChunkHeaderLength:"
                   << chunkHeaderLength << " = " << buffer->byteSize()
                   << std::endl;
  VBuilder builder(*buffer);

  // ****** TODO split data into smaller parts so that a **********************
  //             message can be longer than max chunk len

  // add message header
  addVstMessageHeader(builder, request.header);
  auto slice = VSlice(buffer->data()+chunkHeaderLength);
  auto headerLength = slice.byteSize();
  buffer->resetTo(chunkHeaderLength+headerLength);
  FUERTE_LOG_VSTTRACE << "Message Header:\n" << slice.toJson() << " , "
                                          << chunkHeaderLength << " + " << headerLength
                                          << " = " << buffer->byteSize()
                                          << std::endl;

  // add playload (header + data - uncompressed)
  std::size_t payloadLength = headerLength;
  {
    if(request.header.contentType() == ContentType::VPack){
      for(auto const& slice : request.slices()){
        payloadLength += slice.byteSize();
        FUERTE_LOG_VSTTRACE << to_string(slice) << " , " << "adding: " << slice.byteSize() << " bytes to builder" << std::endl;
        builder.add(slice);
        buffer->resetTo(chunkHeaderLength+payloadLength);
      }
    } else {
      uint8_t const * data;
      std::size_t size;
      std::tie(data,size) = request.payload();
      payloadLength += size;
      buffer->append(data,size);
      buffer->resetTo(chunkHeaderLength+payloadLength);
    }
  }

  // **************************************************************************

  // for the single chunk case chunk len and total message size are the same
  vstChunkHeader.updateChunkPayload(buffer->data(), payloadLength);

  if ((vstChunkHeader._isFirst && vstChunkHeader._numberOfChunks > 1) || (vstVersionID > 1)) {
    vstChunkHeader.updateTotalPayload(buffer->data(), payloadLength);
  }

  //std::cout << "updated header read back in" << std::endl;
  //auto readheader = readChunkHeaderV1_0(buffer.get()->data());
  //FUERTE_LOG_DEBUG << chunkHeaderToString(readheader) << std::endl;

  FUERTE_LOG_VSTTRACE << buffer->byteSize() << " = "
                      << payloadLength << " + "
                      << chunkHeaderLength
                      << std::endl;
  FUERTE_LOG_VSTTRACE << "asserting - buffersize: " << buffer->byteSize()
                      << " == "
                      << "chunkLength: "  << vstChunkHeader._chunkLength
                      << std::endl;

  assert(buffer->byteSize() == vstChunkHeader._chunkLength);
  return buffer;
}

///////////////////////////////////////////////////////////////////////////////////
// receiving vst
///////////////////////////////////////////////////////////////////////////////////

std::size_t isChunkComplete(uint8_t const * const begin, std::size_t const lengthAvailalbe) {
  if (lengthAvailalbe < sizeof(uint32_t)) { // there is not enought to read the length of
    return 0;
  }
  // read chunk length
  uint32_t lengthThisChunk;
  // TODO -- fix endianess
  std::memcpy(&lengthThisChunk, begin, sizeof(uint32_t));
  if (lengthAvailalbe < lengthThisChunk) {
    FUERTE_LOG_VSTTRACE << "\nchunk incomplete: " << lengthAvailalbe << "/" << lengthThisChunk << "(available/len)" << std::endl;
    return 0;
  }
  FUERTE_LOG_VSTTRACE << "\nchunk complete: " << lengthThisChunk << " bytes" << std::endl;
  return lengthThisChunk;
}


ChunkHeader readChunkHeaderV1_0(uint8_t const * const bufferBegin) {
  // TODO -- fix endianess
  ChunkHeader header;

  auto cursor = bufferBegin;

  std::memcpy(&header._chunkLength, cursor, sizeof(header._chunkLength));
  cursor += sizeof(header._chunkLength);

  std::memcpy(&header._chunk, cursor, sizeof(header._chunk));
  cursor += sizeof(header._chunk);

  header._isFirst = header._chunk & 0x1;
  header._numberOfChunks = header._chunk;
  header._numberOfChunks >>= 1;
  header._isSingle = header._isFirst && header._numberOfChunks == 1;

  std::memcpy(&header._messageID, cursor, sizeof(header._messageID));
  cursor += sizeof(header._messageID);

  // extract total len of message
  if (header._isFirst && !header._isSingle) {
    std::memcpy(&header._totalMessageLength, cursor, sizeof(header._totalMessageLength));
    cursor += sizeof(header._totalMessageLength);
  } else {
    header._totalMessageLength = 0;  // not needed
  }

  header._chunkHeaderLength = std::distance(bufferBegin, cursor);
  header._chunkPayloadLength = header._chunkLength - header._chunkHeaderLength;
  return header;
}

MessageHeader messageHeaderFromSlice(int vstVersionID, VSlice const& headerSlice){
  assert(headerSlice.isArray());
  MessageHeader header;
  header.byteSize = headerSlice.byteSize(); //for debugging

  if(vstVersionID == 1){
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
      header.parameter = sliceToStringMap(headerSlice.at(5));                       // params
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
  } catch (std::exception const& e) {
    throw std::runtime_error(std::string("error during validation of incoming VPack: ") + e.what());
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
  while (length) {
    try {
      // isSubPart allows the slice to be shorter than the checked buffer.
      validator.validate(cursor, length , isSubPart);
      VSlice slice(cursor);
      auto sliceSize = slice.byteSize();
      if(length < sliceSize){
        throw std::length_error("slice is longer than buffer");
      }
      length -= sliceSize;
      cursor += sliceSize;
      numPayloads++;
      FUERTE_LOG_VSTTRACE << sliceSize << " ";
    } catch (std::exception const& e) {
      throw std::runtime_error(std::string("error during validation of incoming VPack ") + e.what());
    }
  }
  FUERTE_LOG_VSTTRACE << std::endl;
  return numPayloads;
}

}}}}
