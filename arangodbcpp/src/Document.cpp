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

#include "arangodbcpp/Document.h"

#include "arangodbcpp/Database.h"
#include "arangodbcpp/Collection.h"

namespace arangodb {

namespace dbinterface {

Document::Document(const std::string& inp) : _key(inp) {}

Document::Document(std::string&& inp) : _key(inp) {}

Document::~Document() {}

void Document::addKeyAttrib(arangodb::velocypack::Builder& builder) {
  builder.add("_key", arangodb::velocypack::Value(_key));
}

void Document::syncQuery(ConnectionBase& conn, const Options& opts) {
  typedef Options::Sync Sync;
  switch (opts.flag<Sync>()) {
    case Sync::NoWait: {
      conn.addQuery(ConOption("waitForSync", "false"));
      break;
    }
    case Sync::Wait: {
      conn.addQuery(ConOption("waitForSync", "true"));
      break;
    }
    default:;
  }
}

void Document::mergeQuery(ConnectionBase& conn, const Options& opts) {
  if (opts.flagged(Options::Merge::No)) {
    conn.addQuery(ConOption("mergeObjects", "false"));
  }
}

void Document::keepNullQuery(ConnectionBase& conn, const Options& opts) {
  if (opts.flagged(Options::RemoveNull::Yes)) {
    conn.addQuery(ConOption{"KeepNull", "false"});
  }
}

//
// Configure to create a new empty document with the location determined by the
// pCol parameter
//
void Document::create(const Collection::SPtr& pCol,
                      const Connection::SPtr& pCon, const Options& opts) {
  typedef Connection::Url Url;
  ConnectionBase& conn = pCon->reset();
  Url url{pCol->refColUrl()};
  std::string json{"{\"_key\":\"" + _key + "\"}"};
  { conn.setHeaderOpts(); }
  {
    syncQuery(conn, opts);
    conn.setUrl(url);
  }
  conn.setPostField(json);
  conn.setPostReq();
  conn.setBuffer();
}

void Document::create(const Collection::SPtr& pCol,
                      const Connection::SPtr& pCon,
                      const Connection::VPack data, const Options& opts) {
  typedef Connection::Url Url;
  ConnectionBase& conn = pCon->reset();
  Url url{pCol->refColUrl()};
  { conn.setHeaderOpts(); }
  {
    syncQuery(conn, opts);
    conn.setUrl(url);
  }
  conn.setPostField(data);
  conn.setPostReq();
  conn.setBuffer();
}

//
// Configure to delete a document with the set key name in the
// Database/Collection
// location determined by the pCol parameter
//
void Document::remove(const Collection::SPtr& pCol,
                      const Connection::SPtr& pCon, const Options& opts) {
  ConnectionBase& conn = pCon->reset();
  Connection::Url url = pCol->refDocUrl(_key);
  {
    revMatch(conn, opts);
    conn.setHeaderOpts();
  }
  {
    syncQuery(conn, opts);
    conn.setUrl(url);
  }
  conn.setDeleteReq();
  conn.setBuffer();
}

void Document::revMatch(ConnectionBase& conn, const Options& opts) {
  const std::string& rev = opts;
  if (!rev.empty() && opts.flagged(Options::Rev::Match)) {
    ConOption opt{"If-Match", '"' + rev + '"'};
    conn.addHeader(opt);
  }
}

//
//  Ensures Match _rev headers are mutually exclusive and require
//  an ETag
//
void Document::matchHeader(ConnectionBase& conn, const Options& opts) {
  typedef Options::Rev Rev;
  const std::string& rev = opts;
  if (!rev.empty()) {
    switch (opts.flag<Rev>()) {
      case Rev::NoMatch: {
        ConOption opt{"If-None-Match", '"' + rev + '"'};
        conn.addHeader(opt);
        break;
      }
      case Rev::Match: {
        ConOption opt{"If-Match", '"' + rev + '"'};
        conn.addHeader(opt);
        break;
      }
      default:;
    }
  }
}

//
// Configure to get a document with the set key name in the
// Database/Collection with options
//
// Location determined by the pCol parameter
// DONE
//
void Document::get(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                   const Options& opts) {
  ConnectionBase& conn = pCon->reset();
  matchHeader(conn, opts);
  conn.setUrl(pCol->refDocUrl(_key));
  conn.setGetReq();
  conn.setBuffer();
  conn.setHeaderOpts();
}

//
// Replace a Document with new values in VelocyPack data
//
// All options implemented, match rev done as a query
//
void Document::replace(const Collection::SPtr& pCol,
                       const Connection::SPtr& pCon, Connection::VPack data,
                       const Options& opts) {
  ConnectionBase& conn = pCon->reset();
  Connection::Url url = pCol->refDocUrl(_key);
  {
    revMatch(conn, opts);
    conn.setHeaderOpts();
  }
  {
    syncQuery(conn, opts);
    conn.setUrl(url);
  }
  conn.setPostField(data);
  conn.setPutReq();
  conn.setBuffer();
}

//
// DONE
//
void Document::patch(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                     Connection::VPack data, const Options& opts) {
  ConnectionBase& conn = pCon->reset();
  Connection::Url url{pCol->refDocUrl(_key)};
  {
    revMatch(conn, opts);
    conn.setHeaderOpts();
  }
  {
    syncQuery(conn, opts);
    mergeQuery(conn, opts);
    keepNullQuery(conn, opts);
    conn.setUrl(url);
  }
  conn.setPatchReq();
  conn.setPostField(data);
  conn.setBuffer();
}

Connection::VPack Document::head(bool bSort, const Connection::SPtr& pCon) {
  namespace VTest = arangodb::velocypack;
  using VTest::ValueType;
  using VTest::Value;
  using VTest::Options;
  using VTest::Builder;
  typedef std::string::size_type sz_type;
  auto processLine = [](const std::string line, Builder& build) -> void {
    sz_type split = line.find_first_of(':');
    if (split == std::string::npos) {
      return;
    }
    std::string name = line.substr(0, split);
    split = line.find_first_not_of("\t\r\n ", split + 1);
    std::string value = line.substr(split, line.length() - split);
    if (std::isdigit(value[0])) {
      sz_type num = std::stoul(value);
      build.add(name, Value(num));
      return;
    }
    if (value[0] == '"') {
      split = value.rfind('"');
      value = value.substr(1, split - 1);
    }
    build.add(name, Value(value));
  };
  Options opts;
  opts.sortAttributeNames = bSort;
  Builder build{&opts};
  static const std::string delimEol{"\r\n"};
  std::string res = pCon->bufString();
  sz_type old = 0;
  sz_type pos = res.find_first_of(delimEol, old);
  // Start building an object
  build.add(Value(ValueType::Object));
  for (; pos != std::string::npos; pos = res.find_first_of(delimEol, old)) {
    if (pos != old) {
      processLine(res.substr(old, pos - old), build);
    }
    old = pos + 1;
  }
  pos = res.length();
  if (old != pos) {
    processLine(res.substr(old, pos - old), build);
  }
  build.close();
  return build.steal();
}

//
// DONE
//
void Document::head(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                    const Options& opts) {
  ConnectionBase& conn = pCon->reset();
  Connection::Url url = pCol->refDocUrl(_key);
  {
    matchHeader(conn, opts);
    conn.setHeaderOpts();
  }
  conn.setHeadReq();
  conn.setUrl(url);
  conn.setBuffer();
}
}
}
