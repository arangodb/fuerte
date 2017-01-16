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
static void addHeader(VBuffer& buffer, VBuffer const& header, std::size_t payloadLen){
  std::logic_error("implement me");
}

std::shared_ptr<VBuffer> toNetwork(Request const& request){
  std::stringstream ss;
  //overview
  //
  // - set all required header fileds
  // - create vpack representation of header
  // - a list of vpacks representing the payload
  // - add vst header

  VBuffer headerBuffer;
  VBuilder builder(headerBuffer);
  builder.isOpenObject();
  builder.close(); //close object

  auto buffer = std::make_shared<VBuffer>();
 // std::size_t payloadLen = 0;
 // for(auto const& pbuff : request.payload()){
 //  payloadLen += pbuff.byteSize();
 // }

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


Header readHeaderV1_0(uint8_t const * const bufferBegin) {
  Header header;

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



std::unique_ptr<Response> fromNetwork( NetBuffer const& indata
                                     , std::map<MessageID,std::shared_ptr<RequestItem>>& map
                                     , std::mutex& mapMutex
                                     , std::size_t& consumed
                                     , bool& complete
                                     )
{
  ReadBufferInfo info;
  auto vstheader = readHeaderV1_0(reinterpret_cast<uint8_t const*>(indata.data())); //evilcast

  //implement and rewrite
  //return a complete buffer


  complete = true;
  consumed = indata.size();
  auto response = createResponse(200);

  VBuffer buffer;
  VBuilder builder;
  builder.add(VValue("this is a fake response"));
  response->addVPack(std::move(buffer));

  return std::move(response);
};



}}}}
