////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////

#ifndef FUERTE_DOCUMENT_H
#define FUERTE_DOCUMENT_H 1

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
  void create(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
              const Options& opts = Options{});
  static void create(const Collection::SPtr& pCol,
                     const Connection::SPtr& pConn,
                     const Connection::VPack data,
                     const Options& opts = Options{});
  void remove(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
              const Options& opts = Options{});
  void get(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
           const Options& opts = Options{});
  void head(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
            const Options& opts = Options{});
  static Connection::VPack head(const bool bSort, const Connection::SPtr& pCon);
  void patch(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
             Connection::VPack data, const Options& opts = Options{});
  void replace(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
               Connection::VPack data, const Options& opts = Options{});

  void addKeyAttrib(arangodb::velocypack::Builder& builder);

  Document& operator=(const std::string&);
  Document& operator=(std::string&&);
  operator const std::string&() const;

 private:
  static void syncQuery(ConnectionBase& conn, const Options& opts);
  static void mergeQuery(ConnectionBase& conn, const Options& opts);
  static void keepNullQuery(ConnectionBase& conn, const Options& opts);
  static void revMatch(ConnectionBase& conn, const Options& opts);
  static void matchHeader(ConnectionBase& conn, const Options& opts);

  std::string _key;
};

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

#endif
