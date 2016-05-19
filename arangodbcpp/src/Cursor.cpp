#include <sstream>

#include <velocypack/Slice.h>

#include "arangodbcpp/Cursor.h"

namespace arangodb {

namespace dbinterface {

std::ostream& operator<<(std::ostream& os, Cursor::CacheMode mode) {
  static const std::string ref[3]{"on", "off", "demand"};
  return os << ref[static_cast<uint8_t>(mode)];
}

ConnectionBase::Url Cursor::aqlFncUrl() const {
  return _database->databaseUrl("/_api/aqlfunction");
}

ConnectionBase::Url Cursor::cursorUrl() const {
  return _database->databaseUrl("/_api/cursor");
}

ConnectionBase::Url Cursor::cacheUrl() const {
  return _database->databaseUrl("/_api/query-cache");
}

ConnectionBase::Url Cursor::cachePropsUrl() const {
  return cacheUrl() + std::string{"/properties"};
}

void Cursor::setCacheProps(const ConnectionBase::SPtr& pCon, CacheMode mode,
                           uint16_t max, bool bAsync) {
  std::ostringstream os;
  ConnectionBase& conn = pCon->reset();
  os << "{\"mode\":\"" << mode << "\",\"maxResults\":" << max << ')';
  conn.setUrl(cachePropsUrl());
  conn.setPostField(os.str());
  conn.setPutReq();
  conn.setBuffer();
  conn.setAsynchronous(bAsync);
}

void Cursor::cacheProperties(const ConnectionBase::SPtr& pCon,
                             const bool bAsync) {
  ConnectionBase& conn = pCon->reset();
  conn.setUrl(cachePropsUrl());
  conn.setGetReq();
  conn.setBuffer();
  conn.setAsynchronous(bAsync);
}

void Cursor::clearCache(const ConnectionBase::SPtr& pCon, const bool bAsync) {
  ConnectionBase& conn = pCon->reset();
  conn.setUrl(cacheUrl());
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setAsynchronous(bAsync);
}

void Cursor::remove(const ConnectionBase::SPtr& pCon, std::string id,
                    const bool bAsync) {
  ConnectionBase& conn = pCon->reset();
  conn.setUrl(cursorUrl() + ('/' + id));
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setAsynchronous(bAsync);
}

void Cursor::more(const ConnectionBase::SPtr& pCon, std::string id,
                  const bool bAsync) {
  ConnectionBase& conn = pCon->reset();
  conn.setUrl(cursorUrl() + ('/' + id));
  conn.setPutReq();
  conn.setBuffer();
  conn.setAsynchronous(bAsync);
}

void Cursor::create(const ConnectionBase::SPtr& pCon, const std::string query,
                    uint16_t batSize, const bool bAsync) {
  std::ostringstream os;
  ConnectionBase& conn = pCon->reset();
  conn.setUrl(cursorUrl());
  conn.setPostReq();
  os << "{\"query\" : \"" << query << "\"";
  if (batSize) {
    os << ", \"count\":true, \"batchSize\" : " << batSize;
  }
  os << " }";
  conn.setPostField(os.str());
  conn.setBuffer();
  conn.setAsynchronous(bAsync);
}

void Cursor::create(const ConnectionBase::SPtr& pCon,
                    const ConnectionBase::VPack& config, const bool bAsync) {
  ConnectionBase& conn = pCon->reset();
  conn.setUrl(cursorUrl());
  conn.setPostField(config);
  conn.setBuffer();
  conn.setAsynchronous(bAsync);
}

void Cursor::addFnc(const ConnectionBase::SPtr& pCon, const std::string& name,
                    const std::string& code, const bool bAsync) {
  ConnectionBase& conn = pCon->reset();
  std::string config = " { \"name\":\"" + name;
  config += "\",\"code\":\"" + code;
  config += "\"}";
  conn.setUrl(aqlFncUrl());
  conn.setPostField(config);
  conn.setBuffer();
  conn.setAsynchronous(bAsync);
}

void Cursor::deleteFnc(const ConnectionBase::SPtr& pCon,
                       const std::string& name, const bool bAsync) {
  ConnectionBase& conn = pCon->reset();
  conn.setUrl(aqlFncUrl() + ('/' + name));
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setAsynchronous(bAsync);
}

void Cursor::getFncs(const ConnectionBase::SPtr& pCon, const bool bAsync) {
  ConnectionBase& conn = pCon->reset();
  conn.setUrl(aqlFncUrl());
  conn.setGetReq();
  conn.setBuffer();
  conn.setAsynchronous(bAsync);
}

std::string Cursor::moreId(const ConnectionBase::VPack& res) {
  using arangodb::velocypack::Slice;
  using arangodb::velocypack::ValueLength;
  Slice slice{res->data()};
  do {
    Slice sTmp = slice.get("hasMore");
    if (!sTmp.isBool()) {
      break;
    }
    if (!sTmp.getBool()) {
      break;
    }
    sTmp = slice.get("id");
    if (sTmp.isString()) {
      ValueLength len = sTmp.getStringLength();
      const char* pData = sTmp.getString(len);
      return std::string(pData, len);
    }
  } while (false);
  return std::string{};
}
}
}
