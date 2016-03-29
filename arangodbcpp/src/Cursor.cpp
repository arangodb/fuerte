#include <sstream>

#include <velocypack/Slice.h>

#include "arangodbcpp/Server.h"
#include "arangodbcpp/Cursor.h"

namespace arangodb {

namespace dbinterface {

std::ostream& operator<<(std::ostream& os, Cursor::CacheMode mode) {
  static const std::string ref[3]{"on", "off", "demand"};
  return os << ref[static_cast<uint8_t>(mode)];
}

std::string Cursor::httpAqlFnc() const {
  return _database->databaseUrl() + "/_api/aqlfunction";
}

std::string Cursor::httpCursorUrl() const {
  return _database->databaseUrl() + "/_api/cursor";
}

std::string Cursor::httpCacheUrl() const {
  return _database->databaseUrl() + "/_api/query-cache";
}

std::string Cursor::httpCachePropsUrl() const {
  return httpCacheUrl() + "/properties";
}

void Cursor::httpSetCacheProps(const Connection::SPtr& pCon, CacheMode mode,
                               uint16_t max, bool bAsync) {
  std::ostringstream os;
  Connection& conn = pCon->reset();
  os << "{\"mode\":\"" << mode << "\",\"maxResults\":" << max << ')';
  conn.setUrl(httpCachePropsUrl());
  conn.setPostField(os.str());
  conn.setPutReq();
  conn.setBuffer();
  conn.setSync(bAsync);
}

void Cursor::httpCacheProperties(const Connection::SPtr& pCon,
                                 const bool bAsync) {
  Connection& conn = pCon->reset();
  conn.setUrl(httpCachePropsUrl());
  conn.setGetReq();
  conn.setBuffer();
  conn.setSync(bAsync);
}

void Cursor::httpClearCache(const Connection::SPtr& pCon, const bool bAsync) {
  Connection& conn = pCon->reset();
  conn.setUrl(httpCacheUrl());
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setSync(bAsync);
}

void Cursor::httpDelete(const Connection::SPtr& pCon, std::string id,
                        const bool bAsync) {
  Connection& conn = pCon->reset();
  conn.setUrl(httpCursorUrl() + '/' + id);
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setSync(bAsync);
}

void Cursor::httpMore(const Connection::SPtr& pCon, std::string id,
                      const bool bAsync) {
  Connection& conn = pCon->reset();
  conn.setUrl(httpCursorUrl() + '/' + id);
  conn.setPutReq();
  conn.setBuffer();
  conn.setSync(bAsync);
}

void Cursor::httpCreate(const Connection::SPtr& pCon, const std::string query,
                        uint16_t batSize, const bool bAsync) {
  std::ostringstream os;
  Connection& conn = pCon->reset();
  conn.setUrl(httpCursorUrl());
  conn.setPostReq();
  os << "{\"query\" : \"" << query << "\"";
  if (batSize) {
    os << ", \"count\":true, \"batchSize\" : " << batSize;
  }
  os << " }";
  conn.setPostField(os.str());
  conn.setBuffer();
  conn.setSync(bAsync);
}

void Cursor::httpCreate(const Connection::SPtr& pCon,
                        const Connection::VPack& config, const bool bAsync) {
  Connection& conn = pCon->reset();
  conn.setUrl(httpCursorUrl());
  conn.setPostField(config);
  conn.setBuffer();
  conn.setSync(bAsync);
}

void Cursor::httpAddFnc(const Connection::SPtr& pCon, const std::string& name,
                        const std::string& code, const bool bAsync) {
  Connection& conn = pCon->reset();
  std::string config = " { \"name\":\"" + name;
  config += "\",\"code\":\"" + code;
  config += "\"}";
  conn.setUrl(httpAqlFnc());
  conn.setPostField(config);
  conn.setBuffer();
  conn.setSync(bAsync);
}

void Cursor::httpDeleteFnc(const Connection::SPtr& pCon,
                           const std::string& name, const bool bAsync) {
  Connection& conn = pCon->reset();
  conn.setUrl(httpAqlFnc() + '/' + name);
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setSync(bAsync);
}

void Cursor::httpGetFncs(const Connection::SPtr& pCon, const bool bAsync) {
  Connection& conn = pCon->reset();
  conn.setUrl(httpAqlFnc());
  conn.setGetReq();
  conn.setBuffer();
  conn.setSync(bAsync);
}

std::string Cursor::moreId(const Connection::VPack& res) {
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
