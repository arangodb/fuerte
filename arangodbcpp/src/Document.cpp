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

Connection::QueryPrefix Document::httpSyncQuery(
    std::string& url, const Options& opts,
    const Connection::QueryPrefix prefix) {
  typedef Options::Sync Sync;
  switch (opts.flag<Sync>()) {
    case Sync::NoWait: {
      url += prefix + "waitForSync=false";
      break;
    }
    case Sync::Wait: {
      url += prefix + "waitForSync=true";
      break;
    }
    default: { return prefix; }
  }
  return Connection::QueryPrefix::Next;
}

Connection::QueryPrefix Document::httpMergeQuery(
    std::string& url, const Options& opts,
    const Connection::QueryPrefix prefix) {
  if (opts.flagged(Options::Merge::No)) {
    url += prefix + "mergeObjects=false";
    return Connection::QueryPrefix::Next;
  }
  return prefix;
}

Connection::QueryPrefix Document::httpKeepNullQuery(
    std::string& url, const Options& opts,
    const Connection::QueryPrefix prefix) {
  if (opts.flagged(Options::RemoveNull::Yes)) {
    url += prefix + "KeepNull=false";
    return Connection::QueryPrefix::Next;
  }
  return prefix;
}

//
// Configure to create a new empty document with the location determined by the
// pCol parameter
//
void Document::httpCreate(const Collection::SPtr& pCol,
                          const Connection::SPtr& pCon, const std::string json,
                          const Options& opts) {
  Connection& conn = *pCon;
  // std::string url{pCol->docColUrl()};
  // httpSyncQuery(url, opts);
  std::string url{pCol->refColUrl()};
  httpSyncQuery(url, opts, Connection::QueryPrefix::First);
  conn.reset();
  conn.setUrl(url);
  conn.setPostField(json);
  conn.setPostReq();
  conn.setBuffer();
  conn.setSync(opts.flagged(Options::Run::Async));
}

//
// Configure to delete a document with the set key name in the
// Database/Collection
// location determined by the pCol parameter
//
void Document::httpDelete(const Collection::SPtr& pCol,
                          const Connection::SPtr& pCon, const Options& opts) {
  typedef Connection::QueryPrefix QueryPrefix;
  Connection& conn = pCon->reset();
  std::string url = pCol->refDocUrl(_key);
  httpSyncQuery(url, opts, QueryPrefix::First);
  {
    Connection::HttpHeaderList hdrs;
    httpRevMatch(hdrs, opts);
    if (!hdrs.empty()) {
      conn.setHeaderOpts(hdrs);
    }
  }
  conn.setUrl(url);
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setSync(opts.flagged(Options::Run::Async));
}

void Document::httpRevMatch(Connection::HttpHeaderList& hdrs,
                            const Options& opts) {
  const std::string& rev = opts;
  if (!rev.empty() && opts.flagged(Options::Rev::Match)) {
    std::string opt{"If-Match:\"" + rev + '"'};
    hdrs.push_back(opt);
  }
}

void Document::httpMatchHeader(Connection::HttpHeaderList& hdrs,
                               const Options& opts) {
  typedef Options::Rev Rev;
  const std::string& rev = opts;
  const Rev revOpt = opts.flag<Rev>();
  if (!rev.empty() && revOpt != Rev::Reset) {
    std::string opt{"If"};
    if (revOpt == Rev::NoMatch) {
      opt += "-None";
    }
    opt += "-Match:\"" + rev + '"';
    hdrs.push_back(opt);
  }
}

//
// Configure to get a document with the set key name in the
// Database/Collection with options
//
// Location determined by the pCol parameter
// DONE
//
void Document::httpGet(const Collection::SPtr& pCol,
                       const Connection::SPtr& pCon, const Options& opts) {
  Connection& conn = pCon->reset();
  Connection::HttpHeaderList headers;
  conn.httpProtocol(headers);
  httpMatchHeader(headers, opts);
  conn.setUrl(pCol->refDocUrl(_key));
  conn.setGetReq();
  conn.setBuffer();
  if (!headers.empty()) {
    conn.setHeaderOpts(headers);
  }
  conn.setSync(opts.flagged(Options::Run::Async));
}

//
// Replace a Document with new values in VelocyPack data
//
// All options implemented, match rev done as a query
//
void Document::httpReplace(const Collection::SPtr& pCol,
                           const Connection::SPtr& pCon, Connection::VPack data,
                           const Options& opts) {
  typedef Connection::QueryPrefix QueryPrefix;
  Connection& conn = pCon->reset();
  std::string url = pCol->refDocUrl(_key);
  httpSyncQuery(url, opts, QueryPrefix::First);
  {
    Connection::HttpHeaderList hdrs;
    httpRevMatch(hdrs, opts);
    if (!hdrs.empty()) {
      conn.setHeaderOpts(hdrs);
    }
  }
  conn.setUrl(url);
  conn.setPostField(Connection::json(data, false));
  conn.setPutReq();
  conn.setBuffer();
  conn.setSync(opts.flagged(Options::Run::Async));
}

//
// DONE
//
void Document::httpPatch(const Collection::SPtr& pCol,
                         const Connection::SPtr& pCon, Connection::VPack data,
                         const Options& opts) {
  typedef Connection::QueryPrefix Prefix;
  Connection& conn = pCon->reset();
  std::string url{pCol->refDocUrl(_key)};
  Prefix pre = httpSyncQuery(url, opts, Prefix::First);
  pre = httpMergeQuery(url, opts, pre);
  {
    Connection::HttpHeaderList hdrs;
    httpRevMatch(hdrs, opts);
    if (!hdrs.empty()) {
      conn.setHeaderOpts(hdrs);
    }
  }
  httpKeepNullQuery(url, opts, pre);
  conn.setPatchReq();
  conn.setUrl(url);
  conn.setPostField(Connection::json(data, false));
  conn.setBuffer();
  conn.setSync(opts.flagged(Options::Run::Async));
}

Connection::VPack Document::httpHead(bool bSort, const Connection::SPtr& pCon) {
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
void Document::httpHead(const Collection::SPtr& pCol,
                        const Connection::SPtr& pCon, const Options& opts) {
  Connection& conn = pCon->reset();
  std::string url = pCol->refDocUrl(_key);
  Connection::HttpHeaderList headers;
  conn.httpProtocol(headers);
  httpMatchHeader(headers, opts);
  conn.setHeadReq();
  conn.setUrl(url);
  conn.setBuffer();
  if (!headers.empty()) {
    conn.setHeaderOpts(headers);
  }
  conn.setSync(opts.flagged(Options::Run::Async));
}
}
}
