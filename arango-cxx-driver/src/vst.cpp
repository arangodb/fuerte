#include <fuerte/types.h>
#include <fuerte/vst.h>
#include <velocypack/Validator.h>
#include <sstream>

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {

using VValidator = ::arangodb::velocypack::Validator;
///////////////////////////////////////////////////////////////////////////////////
// sending vst
///////////////////////////////////////////////////////////////////////////////////


// ### not exported ###############################################################
// sending vst

// section - VstChunkHeader
static ChunkHeader createChunkHeader(int vstVersionID
                                    ,MessageID messageID
                                    ,std::size_t totalMessageLength
                                    ,std::size_t chunkPayloadLength
                                    ,std::size_t chunk
                                    ,bool isFirst){

  ChunkHeader header;
  header.messageID = messageID;
  header.totalMessageLength = totalMessageLength; //total payload length
  header.isFirst = isFirst;
  header.isSingle = isFirst && (chunk == 1);
  header.chunk = chunk;
  header.chunkHeaderLength  = chunkHeaderLength(vstVersionID, header.isFirst);
  header.chunkLength = header.chunkHeaderLength+chunkPayloadLength;
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
std::size_t appendToBuffer(VBuffer& buffer, T& value) {
  constexpr std::size_t len = sizeof(T);
  uint8_t charArray[len];
  uint8_t* charPtr = charArray;
  std::memcpy(&charArray, &value, len);
  buffer.append(charPtr, len);
  return len;
}

static std::size_t addVstChunkHeader(std::size_t vstVersionID
                                    ,VBuffer& buffer
                                    ,ChunkHeader& header)
{
  auto chunk = header.chunk;
  chunk <<= 1;
  chunk |= header.isFirst ? 0x1 : 0x0;

  appendToBuffer(buffer, header.chunkLength);
  appendToBuffer(buffer, header.chunk);
  appendToBuffer(buffer, header.messageID);

  if (header.isFirst || (vstVersionID > 1)) {
    appendToBuffer(buffer, header.totalMessageLength);
  }

  return buffer.byteSize();
}

// section - VstMessageHeader
static std::size_t addVstMessageHeader(VBuilder& builder
                                      ,MessageHeader const& header
                                      ,mapss const& headerStrings)
{
  auto startSize = builder.size();

  builder.openArray();
  builder.add(VValue(header.version.get()));                         // 0
  builder.add(VValue(static_cast<int>(header.type.get())));          // 1
  switch (header.type.get()){
    case MessageType::Authentication:
      //builder.add(VValue(header.encryption.get()));
      builder.add(VValue(header.user.get()));
      builder.add(VValue(header.password.get()));
      break;

    case MessageType::Request:
      builder.add(VValue(header.database.get()));                    // 2
      builder.add(VValue(static_cast<int>(header.restVerb.get())));  // 3
      builder.add(VValue(header.path.get()));                        // 4
      if (header.parameter){
        //header.parameter = header_slice.at(5);                     // 5
      }
      if (header.meta){
        //header.meta = header_slice.at(6);                          // 6
      }
      break;

    case MessageType::Response:
      builder.add(VValue(header.responseCode.get()));                // 2
      break;
    default:
      break;
  }
  builder.close();

  return builder.size() - startSize;
}

// ################################################################################

std::shared_ptr<VBuffer> toNetwork(Request& request){
  auto buffer = std::make_shared<VBuffer>();
  std::size_t vstVersionID = 1;
  VBuilder builder(*buffer);

  // TODO we really need a to network chunk:
  // taking the complete payload, offset into the payload, totoak messageLen
  // number of chunks currentchunk number.

  // add chunk header
  auto vstChunkHeader = createSingleChunkHeader(vstVersionID, request.messageid, 0); //size is unfortunatly unknown
  auto chunkHeaderLength = addVstChunkHeader(std::size_t(1), *buffer, vstChunkHeader);

  // ****** TODO split data into smaller parts so that a **********************
  //             message can be longer than max chunk len

  // add message header
  auto headerLength = addVstMessageHeader(builder, request.header, request.headerStrings);
  // add playload (header + data - uncompressed)
  std::size_t payloadLength = headerLength; // length of:
                                            // header slice + vpack data or
                                            // header slice + binary data
  {
    if(request.header.contentType == ContentType::VPack){
      for(auto const& slice : request.slices()){
        payloadLength += slice.byteSize();
        builder.add(slice);
      }
    } else {
      uint8_t const * data;
      std::size_t size;
      std::tie(data,size) = request.payload();
      payloadLength += size;
      buffer->append(data,size);
    }
  }

  // **************************************************************************

  // for the single chunk case chunk len and total message size are the same
  vstChunkHeader.updateChunkPayload(buffer->data(), payloadLength);
  vstChunkHeader.updateTotalPayload(buffer->data(), payloadLength);
  return std::move(buffer);
}

///////////////////////////////////////////////////////////////////////////////////
// receiving vst
///////////////////////////////////////////////////////////////////////////////////

std::size_t isChunkComplete(uint8_t const * const begin, std::size_t const length) {
  uint32_t lenthThisChunk;
  if (length < sizeof(uint32_t)) { // there is not enought to read the length of
                                   // the chunk
    return 0;
  }
  // read chunk length
  std::memcpy(&lenthThisChunk, begin, sizeof(uint32_t));
  if (length < lenthThisChunk) {
    // chunk not complete
    return 0;
  }
  return lenthThisChunk;
}


ChunkHeader readChunkHeaderV1_0(uint8_t const * const bufferBegin) {
  ChunkHeader header;

  auto cursor = bufferBegin;

  std::memcpy(&header.chunkLength, cursor, sizeof(header.chunkLength));
  cursor += sizeof(header.chunkLength);

  uint32_t chunkX;
  std::memcpy(&chunkX, cursor, sizeof(chunkX));
  cursor += sizeof(chunkX);

  header.isFirst = chunkX & 0x1;
  header.chunk = chunkX >> 1;

  std::memcpy(&header.messageID, cursor, sizeof(header.messageID));
  cursor += sizeof(header.messageID);

  // extract total len of message
  if (header.isFirst && header.chunk > 1) {
    std::memcpy(&header.totalMessageLength, cursor, sizeof(header.totalMessageLength));
    cursor += sizeof(header.totalMessageLength);
  } else {
    header.totalMessageLength = 0;  // not needed
  }

  header.chunkHeaderLength = std::distance(bufferBegin, cursor);
  header.isSingle = header.isFirst && header.chunk == 1;
  return header;
}

MessageHeader messageHeaderFromSlice(VSlice const& headerSlice){
  assert(headerSlice.isArray());
  MessageHeader header;

  header.version = headerSlice.at(0).getNumber<int>(); //version
  header.type = static_cast<MessageType>(headerSlice.at(1).getNumber<int>()); //type
  switch (header.type.get()){
    case MessageType::Authentication:
      //header.encryption = headerSlice.at(6); //encryption (plain) should be 2
      header.user = headerSlice.at(2).copyString(); //user
      header.user = headerSlice.at(3).copyString(); //password
      break;

    case MessageType::Request:
      header.database = headerSlice.at(2).copyString(); // databse
      header.restVerb = static_cast<RestVerb>(headerSlice.at(3).getInt()); //rest verb
      header.path = headerSlice.at(4).copyString();  // request (path)
      //header.parameter = headerSlice.at(5); // params
      //header.parameter = headerSlice.at(6); // meta
      break;

    case MessageType::Response:
      header.responseCode = headerSlice.at(2).getInt();
      break;
    default:
      break;
  }

  return header;
};

MessageHeader validateAndExtractMessageHeader(uint8_t const * const vpStart, std::size_t length, std::size_t& headerLength){
  using VValidator = ::arangodb::velocypack::Validator;
  // there must be at least one velocypack for the header
  VValidator validator;
  bool isSubPart = true;

  VSlice slice;
  try {
    // isSubPart allows the slice to be shorter than the checked buffer.
    validator.validate(vpStart, length , isSubPart);
  } catch (std::exception const& e) {
    throw std::runtime_error(std::string("error during validation of incoming VPack") + e.what());
  }
  slice = VSlice(vpStart);
  headerLength = slice.byteSize();

  return messageHeaderFromSlice(slice);
}

std::size_t validateAndCount(uint8_t const * const vpStart, std::size_t length){
  // start points to the begin of a velocypack
  uint8_t const * cursor = vpStart;
  // there must be at least one velocypack for the header
  VValidator validator;
  std::size_t numPayloads = 0; // fist item is the header
  bool isSubPart = true;

  while (length) {
    try {
      // isSubPart allows the slice to be shorter than the checked buffer.
      validator.validate(cursor, length , isSubPart);
      VSlice slice(cursor);
      length -= slice.byteSize();
      cursor += slice.byteSize();
      numPayloads++;
    } catch (std::exception const& e) {
      throw std::runtime_error(std::string("error during validation of incoming VPack") + e.what());
    }
  }
  return numPayloads;
}

}}}}
