////////////////////////////////////////////////////////////////////////////////
/// @brief C++ Library to interface to Arangodb.
///
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
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
#ifndef FUERTE_DOCUMENT_H
#define FUERTE_DOCUMENT_H

#include <velocypack/Builder.h>

#include "arangodbcpp/Connection.h"
#include "arangodbcpp/Collection.h"
#include "arangodbcpp/DocOptions.h"

namespace arangodb {

namespace dbinterface {

class Document {
 public:
  typedef DocOptions Options;
  typedef std::shared_ptr<Document> SPtr;
  Document(const std::string& name);
  Document(std::string&& name = "NewDoc");
  virtual ~Document();
  void httpCreate(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                  const Options& opts = Options{});
  static void httpCreate(const Collection::SPtr& pCol,
                         const Connection::SPtr& pConn,
                         const Connection::VPack data,
                         const Options& opts = Options{});
  static Connection::VPack httpCreate(const bool bSort,
                                      const Connection::SPtr& pCon);
  void httpDelete(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                  const Options& opts = Options{});
  static Connection::VPack httpDelete(const bool bSort,
                                      const Connection::SPtr& pCon);
  void httpGet(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
               const Options& opts = Options{});
  static Connection::VPack httpGet(const bool bSort,
                                   const Connection::SPtr& pCon);
  void httpHead(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                const Options& opts = Options{});
  static Connection::VPack httpHead(const bool bSort,
                                    const Connection::SPtr& pCon);
  void httpPatch(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                 Connection::VPack data, const Options& opts = Options{});
  static Connection::VPack httpPatch(const bool bSort,
                                     const Connection::SPtr& pCon);
  void httpReplace(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                   Connection::VPack data, const Options& opts = Options{});
  static Connection::VPack httpReplace(const bool bSort,
                                       const Connection::SPtr& pCon);

  void addKeyAttrib(arangodb::velocypack::Builder& builder);

  Document& operator=(const std::string&);
  Document& operator=(std::string&&);
  operator const std::string&() const;

 private:
  static void httpCreate(const Collection::SPtr& pCol,
                         const Connection::SPtr& pCon, const std::string json,
                         const Options& opts);
  static Connection::QueryPrefix httpCreateQuery(
      std::string& url, const Options& opts,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);
  static Connection::QueryPrefix httpSyncQuery(
      std::string& url, const Options& opts,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);
  static Connection::QueryPrefix httpMergeQuery(
      std::string& url, const Options& opts,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);
  static Connection::QueryPrefix httpPolicyQuery(
      std::string& url, const Options& opts,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);
  static Connection::QueryPrefix httpRevQuery(
      std::string& url, const Options& opt,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);

  static Connection::QueryPrefix httpKeepNullQuery(
      std::string& url, const Options& optds,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);

  Connection::QueryPrefix httpCreateTypeQuery(
      std::string& url, const Options& opts,
      const Connection::QueryPrefix prefix);

  static void httpMatchHeader(Connection& conn, const Options& opts);

  std::string _key;
};

inline Connection::VPack Document::httpCreate(const bool bSort,
                                              const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline void Document::httpCreate(const Collection::SPtr& pCol,
                                 const Connection::SPtr& pCon,
                                 const Connection::VPack data,
                                 const Options& opts) {
  httpCreate(pCol, pCon, Connection::json(data, false), opts);
}

inline void Document::httpCreate(const Collection::SPtr& pCol,
                                 const Connection::SPtr& pCon,
                                 const Options& opts) {
  httpCreate(pCol, pCon, std::string{"{\"_key\":\"" + _key + "\"}"}, opts);
}

inline Connection::VPack Document::httpDelete(const bool bSort,
                                              const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Document::httpGet(const bool bSort,
                                           const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Document::httpPatch(const bool bSort,
                                             const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Document::httpReplace(const bool bSort,
                                               const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Document::operator const std::string&() const { return _key; }

inline Document& Document::operator=(const std::string& inp) {
  _key = inp;
  return *this;
}

inline Document& Document::operator=(std::string&& inp) {
  _key = inp;
  return *this;
}
}
}

#endif  // FUERTE_DOCUMENT_H
