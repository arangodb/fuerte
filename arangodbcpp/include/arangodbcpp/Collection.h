////////////////////////////////////////////////////////////////////////////////
/// @brief C++ Library to interface to Arangodb.
///
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
/// @author Copyright 2016, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////
#ifndef FUERTE_COLLECTION_H
#define FUERTE_COLLECTION_H
#include <velocypack/Builder.h>

#include "arangodbcpp/Database.h"
#include "arangodbcpp/Connection.h"

namespace arangodb {

namespace dbinterface {

class Collection {
 public:
  typedef std::shared_ptr<Collection> SPtr;
  class Options {
   public:
    typedef uint8_t Flags;
    enum class Run : Flags {
      Reset = 0,
      Sync = Reset,
      Async = Sync + 1,
      Mask = 1
    };
    enum class List : Flags {
      Reset = 0,
      Path = Reset,
      Id = Path + 2,
      Key = Id + 2,
      Mask = 6
    };
    enum class Revs : Flags { Reset = 0, No = 0, Yes = No + 8, Mask = 8 };
    enum class Data : Flags { Reset = 0, No = 0, Yes = No + 16, Mask = 16 };

   private:
    Flags _flgs;
    template <typename T>
    void addFlags(T flag);

    template <typename T, typename... Args>
    void addFlags(T flag, Args... args);

   public:
    explicit Options();
    template <typename T>
    explicit Options(T val);
    template <typename T, typename... Args>
    explicit Options(T val, Args... args);
    template <typename T>
    Options& setFlags(T val);
    template <typename T, typename... Args>
    Options& setFlags(T val, Args... args);
    template <typename T>
    Options& resetFlags();
    template <typename T>
    const T flag() const;
    template <typename T>
    bool flagged(T inp) const;
  };
  Collection() = delete;
  explicit Collection(const std::shared_ptr<Database>& db,
                      const std::string& id);
  explicit Collection(const std::shared_ptr<Database>& db,
                      std::string&& id = "new-collection");
  ~Collection();

  static void httpCreate(const Database::SPtr& pDb,
                         const Connection::SPtr& pCon,
                         const Connection::VPack& body,
                         const Options opts = Options());
  void httpCreate(const Connection::SPtr& pCon, const Options opts = Options());
  static Connection::VPack httpCreate(const bool bSort,
                                      const Connection::SPtr& pCon);

  void httpDocs(const Connection::SPtr& pCon, const Options opts = Options());
  static Connection::VPack httpDocs(const bool bSort,
                                    const Connection::SPtr& pCon);

  void httpDelete(const Connection::SPtr& pCon, const Options opts = Options());
  static Connection::VPack httpDelete(const bool bSort,
                                      const Connection::SPtr& pCon);

  void httpAbout(const Connection::SPtr& pCon, const Options opts = Options());
  static Connection::VPack httpAbout(const bool bSort,
                                     const Connection::SPtr& pCon);

  void httpTruncate(const Connection::SPtr& pCon,
                    const Options opts = Options());
  static Connection::VPack httpTruncate(const bool bSort,
                                        const Connection::SPtr& pCon);

  void httpRename(const Connection::SPtr& pCon, const std::string& name,
                  const Options opts = Options());
  static Connection::VPack httpRename(const bool bSort,
                                      const Connection::SPtr& pCon);

  void httpCount(const Connection::SPtr& pCon, const Options opts = Options());
  static Connection::VPack httpCount(const bool bSort,
                                     const Connection::SPtr& pCon);

  void httpChecksum(const Connection::SPtr& pCon,
                    const Options opts = Options());
  static Connection::VPack httpChecksum(const bool bSort,
                                        const Connection::SPtr& pCon);

  void httpStats(const Connection::SPtr& pCon, const Options opts = Options());
  static Connection::VPack httpStats(const bool bSort,
                                     const Connection::SPtr& pCon);

  void httpProps(const Connection::SPtr& pCon, const Options opts = Options());
  static Connection::VPack httpProps(const bool bSort,
                                     const Connection::SPtr& pCon);

  void httpRevId(const Connection::SPtr& pCon, const Options opts = Options());
  static Connection::VPack httpRevId(const bool bSort,
                                     const Connection::SPtr& pCon);

  std::string docColUrl() const;
  std::string refDocUrl(const std::string& key) const;
  bool hasValidHost() const;
  Collection& operator=(const std::string&);
  Collection& operator=(std::string&&);
  const std::string id() const;
  void addNameAttrib(arangodb::velocypack::Builder& builder);
  operator const std::string&() const;

 private:
  void httpInfo(const Connection::SPtr& pCon, const Options,
                const std::string&& info);
  const std::string httpApi() const;
  const std::string httpApiName() const;

  std::shared_ptr<Database> _database;
  std::string _name;
};

template <typename T>
inline void Collection::Options::addFlags(T flag) {
  _flgs &= ~static_cast<Flags>(T::Mask);
  _flgs |= static_cast<Flags>(flag);
}

template <typename T, typename... Args>
inline void Collection::Options::addFlags(T flag, Args... args) {
  addFlags(flag);
  addFlags(args...);
}

inline Collection::Options::Options() : _flgs(0) {}

template <typename T>
inline Collection::Options& Collection::Options::setFlags(T val) {
  if (T::Mask == T::Mask) {
  }
  _flgs = static_cast<Flags>(val);
  return *this;
}

template <typename T, typename... Args>
inline Collection::Options& Collection::Options::setFlags(T val, Args... args) {
  if (T::Mask == T::Mask) {
  }
  _flgs = static_cast<Flags>(val);
  addFlags(args...);
  return *this;
}

template <typename T>
inline Collection::Options::Options(T val)
    : _flgs(static_cast<Flags>(val)) {
  if (T::Mask == T::Mask) {
  }
}

template <typename T, typename... Args>
inline Collection::Options::Options(T val, Args... args)
    : _flgs(static_cast<Flags>(val)) {
  addFlags(args...);
}

template <typename T>
inline Collection::Options& Collection::Options::resetFlags() {
  _flgs &= ~static_cast<Flags>(T::Mask);
  return *this;
}

template <typename T>
inline const T Collection::Options::flag() const {
  Flags tmp = _flgs & static_cast<Flags>(T::Mask);
  return static_cast<T>(tmp);
}

template <typename T>
inline bool Collection::Options::flagged(T inp) const {
  return flag<T>() == inp;
}

inline Collection& Collection::operator=(const std::string& inp) {
  _name = inp;
  return *this;
}

inline Collection::operator const std::string&() const { return _name; }

inline Collection& Collection::operator=(std::string&& inp) {
  _name = inp;
  return *this;
}

inline const std::string Collection::id() const { return _name; }

inline Connection::VPack Collection::httpCreate(const bool bSort,
                                                const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Collection::httpDelete(const bool bSort,
                                                const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Collection::httpAbout(const bool bSort,
                                               const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Collection::httpTruncate(
    bool bSort, const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Collection::httpDocs(const bool bSort,
                                              const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

//
// Configure to get document count in a Collection using the configured
// Database and Collection name
//
inline void Collection::httpCount(const Connection::SPtr& pCon,
                                  const Options opts) {
  httpInfo(pCon, opts, "/count");
}

inline Connection::VPack Collection::httpCount(const bool bSort,
                                               const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Collection::httpChecksum(
    const bool bSort, const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline void Collection::httpProps(const Connection::SPtr& pCon,
                                  const Options opts) {
  httpInfo(pCon, opts, "/propeties");
}

inline Connection::VPack Collection::httpProps(const bool bSort,
                                               const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline void Collection::httpStats(const Connection::SPtr& pCon,
                                  const Options opts) {
  httpInfo(pCon, opts, "/figures");
}

inline Connection::VPack Collection::httpStats(const bool bSort,
                                               const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Collection::httpRename(const bool bSort,
                                                const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline void Collection::httpRevId(const Connection::SPtr& pCon,
                                  const Options opts) {
  httpInfo(pCon, opts, "/revision");
}

inline Connection::VPack Collection::httpRevId(const bool bSort,
                                               const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}
}
}

#endif  // FUERTE_COLLECTION_H
