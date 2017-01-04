#include <fuerte/vst.h>

#include <velocypack/Validator.h>

namespace arangodb { namespace fuerte { inline namespace v1 { namespace vst {
using VValidator = ::arangodb::velocypack::Validator;

std::size_t validateAndCount(uint8_t const* vpHeaderStart
                            ,uint8_t const* vpEnd) {
  try {
    VValidator validator;
    // check for slice start to the end of Chunk
    // isSubPart allows the slice to be shorter than the checked buffer.
    validator.validate(vpHeaderStart, std::distance(vpHeaderStart, vpEnd),
                       /*isSubPart =*/true);

    VSlice vpHeader(vpHeaderStart);
    auto vpPayloadStart = vpHeaderStart + vpHeader.byteSize();

    std::size_t numPayloads = 0;
    while (vpPayloadStart != vpEnd) {
      // validate
      validator.validate(vpPayloadStart, std::distance(vpPayloadStart, vpEnd),
                         true);
      // get offset to next
      VSlice tmp(vpPayloadStart);
      vpPayloadStart += tmp.byteSize();
      numPayloads++;
    }
    return numPayloads;
  } catch (std::exception const& e) {
    throw std::runtime_error(
        std::string("error during validation of incoming VPack") + e.what());
  }
}


Header readVstHeader(uint8_t const * const bufferBegin, vst::ReadBufferInfo& info) {
  Header header;

  auto cursor = bufferBegin + info.readBufferOffset;

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

  header.headerLength = std::distance(
      bufferBegin + info.readBufferOffset, cursor);

  return header;
}

std::size_t isChunkComplete(uint8_t const* start, std::size_t length , ReadBufferInfo& info) {
  if (!info.currentChunkLength && length < sizeof(uint32_t)) {
    return 0;
  }
  if (!info.currentChunkLength) {
    // read chunk length
    std::memcpy(&info.currentChunkLength, start, sizeof(uint32_t));
  }
  if (length < info.currentChunkLength) {
    // chunk not complete
    return 0;
  }
  return info.currentChunkLength;
}

}}}}
