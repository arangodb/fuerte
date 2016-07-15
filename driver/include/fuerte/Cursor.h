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
/// @author John Bufton
////////////////////////////////////////////////////////////////////////////////

#ifndef FUERTE_CURSOR_H
#define FUERTE_CURSOR_H 1

#include <fuerte/Database.h>

namespace arangodb {
namespace dbinterface {
class Cursor {
 public:
  typedef std::shared_ptr<Cursor> SPtr;
  enum class CacheMode : uint8_t { on = 0, off, demand };

  Cursor(const Database::SPtr& inp);
  Cursor() = delete;
  virtual ~Cursor();

  void create(const Connection::SPtr& pCon, const std::string query,
              uint16_t batSize = 0, const bool bAsync = false);
  void create(const Connection::SPtr& pCon,
              const Connection::VPack& config, const bool bAsync = false);
  void more(const Connection::SPtr& pCon, std::string id,
            const bool bAsync = false);
  void addFnc(const Connection::SPtr& pCon, const std::string& name,
              const std::string& code, const bool bAsync);
  void deleteFnc(const Connection::SPtr& pCon, const std::string& name,
                 const bool bAsync);
  void getFncs(const Connection::SPtr& pCon, const bool bAsync);
  void remove(const Connection::SPtr& pCon, std::string id,
              const bool bAsync = false);
  void clearCache(const Connection::SPtr& pCon, const bool bAsync = false);
  void cacheProperties(const Connection::SPtr& pCon,
                       const bool bAsync = false);
  void setCacheProps(const Connection::SPtr& pCon, enum CacheMode mode,
                     uint16_t max, const bool bAsync = false);

  static std::string moreId(const Connection::VPack& res);

 private:
  Connection::Url cursorUrl() const;
  Connection::Url aqlFncUrl() const;
  Connection::Url cacheUrl() const;
  Connection::Url cachePropsUrl() const;

  Database::SPtr _database;
};

inline Cursor::~Cursor() {}

inline Cursor::Cursor(const Database::SPtr& inp) : _database(inp) {}

std::ostream& operator<<(std::ostream& os, Cursor::CacheMode mode);
}
}

#endif
