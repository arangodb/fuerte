#ifndef FUERTE_CURSOR_H
#define FUERTE_CURSOR_H

#include "arangodbcpp/Server.h"
#include "arangodbcpp/Connection.h"

namespace arangodb {

namespace dbinterface {

class Cursor {
 public:
  enum class CacheMode : uint8_t { on = 0, off, demand };

  Cursor(const Server::SPtr& inp);
  Cursor() = delete;

  void httpCreate(const Connection::SPtr& pCon, const std::string query,
                  uint16_t batSize = 0, bool bAsync = false);
  void httpCreate(const Connection::SPtr& pCon, const Connection::VPack& config,
                  bool bAsync = false);
  static Connection::VPack httpCreate(bool bSort, const Connection::SPtr& pCon);

  void httpMore(const Connection::SPtr& pCon, std::string id,
                bool bAsync = false);
  static Connection::VPack httpMore(bool bSort, const Connection::SPtr& pCon);

  void httpDelete(const Connection::SPtr& pCon, std::string id,
                  bool bAsync = false);
  static Connection::VPack httpDelete(bool bSort, const Connection::SPtr& pCon);

  void httpClearCache(const Connection::SPtr& pCon, bool bAsync = false);
  static Connection::VPack httpClearCache(bool bSort,
                                          const Connection::SPtr& pCon);

  void httpCacheProperties(const Connection::SPtr& pCon, bool bAsync = false);
  static Connection::VPack httpCacheProperties(bool bSort,
                                               const Connection::SPtr& pCon);

  void httpSetCacheProps(const Connection::SPtr& pCon, enum CacheMode mode,
                         uint16_t max, bool bAsync = false);
  static Connection::VPack httpSetCacheProps(bool bSort,
                                             const Connection::SPtr& pCon);

  static std::string moreId(const Connection::VPack& res);

 private:
  std::string httpCursorUrl() const;
  std::string httpCacheUrl() const;
  std::string httpCachePropsUrl() const;

  Server::SPtr _Server;
};

inline Cursor::Cursor(const Server::SPtr& inp) : _Server(inp) {}

std::ostream& operator<<(std::ostream& os, Cursor::CacheMode mode);

inline Connection::VPack Cursor::httpCreate(bool bSort,
                                            const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Cursor::httpMore(bool bSort,
                                          const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Cursor::httpDelete(bool bSort,
                                            const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Cursor::httpClearCache(bool bSort,
                                                const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Cursor::httpCacheProperties(
    bool bSort, const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Cursor::httpSetCacheProps(
    bool bSort, const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}
}
}

#endif  // FUERTE_CURSOR_H
