
#include <fuerte/old/Connection.h>

#include <velocypack/Dumper.h>
#include <velocypack/Parser.h>
#include <velocypack/Sink.h>

namespace arangodb {

namespace dbinterface {

std::string Connection::json(const VPack& v) {
  using arangodb::velocypack::Slice;
  using arangodb::velocypack::Dumper;
  using arangodb::velocypack::StringSink;
  Slice slice{v->data()};
  std::string tmp;
  StringSink sink(&tmp);
  Dumper::dump(slice, &sink);
  return tmp;
}

Connection::VPack Connection::vpack(const uint8_t* data, std::size_t sz) {
  if (sz < 2) {
    return Connection::VPack{};
  }
  using arangodb::velocypack::Builder;
  using arangodb::velocypack::Parser;
  Parser parser;
  parser.parse(data, sz);
  std::shared_ptr<Builder> vp{parser.steal()};
  return vp->buffer();
}
}
}
