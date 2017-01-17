#include <fuerte/common_types.h>
#include <fuerte/vst.h>
#include <velocypack/Validator.h>
#include <sstream>

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {

using VValidator = ::arangodb::velocypack::Validator;
///////////////////////////////////////////////////////////////////////////////////
// sending vst
///////////////////////////////////////////////////////////////////////////////////

//not exported
static void addVstHeader(VBuilder& builder
                        ,MessageHeader const& header
                        ,mapss const& headerStrings)
{
  std::logic_error("implement me");
}

std::shared_ptr<VBuffer> toNetwork(Request& request){
  std::stringstream ss;

  auto buffer = std::make_shared<VBuffer>();
  // add VstChunkHeader - with length possibly unkown
  VBuilder builder(*buffer);
  //addVstHeader(*buffer, request.header, request.headerStrings);
  // add VstHeader (vpack)
  // add Playload

  //std::size_t payloadLen = 0;
  //{
  //  if(request.header.contentType == ContentType::VPack){
  //    for(auto const& pbuff : request.slices()){
  //      payloadLen += pbuff.byteSize();
  //    }
  //  } else {
  //    payloadLen = request.payload().second;
  //  }
  //}

  builder.isOpenObject();
  builder.close(); //close object

 // addHeader(*buffer, headerBuffer, payloadLen);

 // for(auto const& pbuff : request.payload){
 //  buffer->append(pbuff.data(),pbuff.byteSize());
 // }

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
    std::memcpy(&header.messageLength, cursor, sizeof(header.messageLength));
    cursor += sizeof(header.messageLength);
  } else {
    header.messageLength = 0;  // not needed
  }

  header.headerLength = std::distance(bufferBegin, cursor);
  header.isSingle = header.isFirst && header.chunk == 1;
  return header;
}

MessageHeader messageHeaderFromSlice(VSlice const& headerSlice){
  throw std::logic_error("implement me");
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
