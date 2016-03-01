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

std::string Cursor::httpCursorUrl() const {
  return _Server->hostUrl() + "/_api/cursor";
}

std::string Cursor::httpCacheUrl() const {
  return _Server->hostUrl() + "/_api/query-cache";
}

std::string Cursor::httpCachePropsUrl() const {
  return httpCacheUrl() + "/properties";
}

void Cursor::httpSetCacheProps(const Connection::SPtr& pCon,
                               enum CacheMode mode, uint16_t max, bool bAsync) {
  std::ostringstream os;
  Connection& conn = *pCon;
  os << "{\"mode\":\"" << mode << "\",\"maxResults\":" << max << ')';
  conn.reset();
  conn.setUrl(httpCachePropsUrl());
  conn.setPostField(os.str());
  conn.setPutReq();
  conn.setBuffer();
  conn.setReady(bAsync);
}

void Cursor::httpCacheProperties(const Connection::SPtr& pCon, bool bAsync) {
  Connection& conn = *pCon;
  conn.reset();
  conn.setUrl(httpCachePropsUrl());
  conn.setGetReq();
  conn.setBuffer();
  conn.setReady(bAsync);
}

void Cursor::httpClearCache(const Connection::SPtr& pCon, bool bAsync) {
  Connection& conn = *pCon;
  conn.reset();
  conn.setUrl(httpCacheUrl());
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setReady(bAsync);
}

void Cursor::httpDelete(const Connection::SPtr& pCon, std::string id,
                        bool bAsync) {
  Connection& conn = *pCon;
  conn.reset();
  conn.setUrl(httpCursorUrl() + '/' + id);
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setReady(bAsync);
}

void Cursor::httpMore(const Connection::SPtr& pCon, std::string id,
                      bool bAsync) {
  Connection& conn = *pCon;
  conn.reset();
  conn.setUrl(httpCursorUrl() + '/' + id);
  conn.setPutReq();
  conn.setBuffer();
  conn.setReady(bAsync);
}

void Cursor::httpCreate(const Connection::SPtr& pCon, const std::string query,
                        uint16_t batSize, bool bAsync) {
  std::ostringstream os;
  Connection& conn = *pCon;
  conn.reset();
  conn.setUrl(httpCursorUrl());
  conn.setPostReq();
  os << "{\"query\":\"" << query << '"';
  if (batSize) {
    os << ",\"count\":true,\"batchSize\":" << batSize;
  }
  os << '}';
  conn.setPostField(os.str());
  conn.setBuffer();
  conn.setReady(bAsync);
}

void Cursor::httpCreate(const Connection::SPtr& pCon,
                        const Connection::VPack& config, bool bAsync) {
  Connection& conn = *pCon;
  conn.reset();
  conn.setUrl(httpCursorUrl());
  conn.setPostField(config);
  conn.setBuffer();
  conn.setReady(bAsync);
}

std::string Cursor::moreId(const Connection::VPack& res) {
  using arangodb::velocypack::Slice;
  using arangodb::velocypack::ValueLength;
  Slice slice{res->data()};
  std::string ret;
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
      ret = std::string(pData, len);
    }
  } while (false);
  return ret;
}
}
}
