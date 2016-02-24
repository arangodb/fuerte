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
  typedef DocOptions::Flags Flags;
  typedef std::shared_ptr<Document> SPtr;
  Document(const std::string& name);
  Document(std::string&& name = "NewDoc");
  ~Document();
  void httpCreate(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                  const Options& opts);
  static void httpCreate(const Collection::SPtr& pCol,
                         const Connection::SPtr& pConn,
                         const Connection::VPack data, const Options& opts);
  void httpDelete(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                  const Options& opts);
  void httpGet(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
               const Options& opts = DocOptions());
  void httpHead(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                const Options& opts);
  void httpPatch(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                 const Options& opts, Connection::VPack data);
  void httpReplace(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                   const Options& opts, Connection::VPack data);
  Connection::VPack httpCreate(bool bSort, const Connection::SPtr& pCon);
  Connection::VPack httpDelete(bool bSort, const Connection::SPtr& pCon);
  Connection::VPack httpGet(bool bSort, const Connection::SPtr& pCon);
  Connection::VPack httpHead(bool bSort, const Connection::SPtr& pCon);
  Connection::VPack httpPatch(bool bSort, const Connection::SPtr& pCon);
  void addKeyAttrib(arangodb::velocypack::Builder& builder);

  Document& operator=(const std::string&);
  Document& operator=(std::string&&);
  const std::string key();

 private:
  static void httpCreate(const Collection::SPtr& pCol,
                         const Connection::SPtr& pCon, const std::string json,
                         const Options& opts);
  static Connection::QueryPrefix httpCreateQuery(
      std::string& url, const Flags flgs,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);
  static Connection::QueryPrefix httpSyncQuery(
      std::string& url, const Flags flgs,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);
  static Connection::QueryPrefix httpMergeQuery(
      std::string& url, const Flags flgs,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);
  static Connection::QueryPrefix httpPolicyQuery(
      std::string& url, const Flags flgs,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);
  static Connection::QueryPrefix httpRevQuery(
      std::string& url, const DocOptions& opt,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);
  static Connection::QueryPrefix httpKeepNullQuery(
      std::string& url, const Flags flgs,
      const Connection::QueryPrefix = Connection::QueryPrefix::Next);

  static void httpMatchHeader(Connection::HttpHeaderList& headers,
                              const DocOptions& opts);

  std::string _key;
};

inline Connection::VPack Document::httpCreate(bool bSort,
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

inline Connection::VPack Document::httpDelete(bool bSort,
                                              const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Document::httpGet(bool bSort,
                                           const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Connection::VPack Document::httpPatch(bool bSort,
                                             const Connection::SPtr& pCon) {
  return pCon->fromJSon(bSort);
}

inline Document& Document::operator=(const std::string& inp) {
  _key = inp;
  return *this;
}

inline Document& Document::operator=(std::string&& inp) {
  _key = inp;
  return *this;
}

inline const std::string Document::key() { return _key; }
}
}

#endif  // FUERTE_DOCUMENT_H
