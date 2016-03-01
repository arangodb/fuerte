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

#include "arangodbcpp/Database.h"
#include "arangodbcpp/Collection.h"
#include "arangodbcpp/Document.h"

namespace arangodb {

namespace dbinterface {

Document::Document(const std::string& inp) : _key(inp) {}

Document::Document(std::string&& inp) : _key(inp) {}

Document::~Document() {}

void Document::addKeyAttrib(arangodb::velocypack::Builder& builder) {
  builder.add("_key", arangodb::velocypack::Value(_key));
}

Connection::QueryPrefix Document::httpSyncQuery(
    std::string& url, const Flags flgs, const Connection::QueryPrefix prefix) {
  switch (flgs & Options::Opt_SyncMask) {
    case Options::Opt_NoWaitForSync: {
      url += prefix + "waitForSync=false";
      break;
    }
    case Options::Opt_WaitForSync: {
      url += prefix + "waitForSync=true";
      break;
    }
    default: { return prefix; }
  }
  return Connection::QueryPrefix::Next;
}

//
// Note : default policy is error so not needed
//
Connection::QueryPrefix Document::httpPolicyQuery(
    std::string& url, const Flags flgs, const Connection::QueryPrefix prefix) {
  if (flgs & Options::Opt_PolicyLast) {
    url += prefix + "policy=last";
    return Connection::QueryPrefix::Next;
  }
  return prefix;
}

Connection::QueryPrefix Document::httpCreateQuery(
    std::string& url, const Flags flgs, const Connection::QueryPrefix prefix) {
  if (flgs & Options::Opt_CreateCollection) {
    url += prefix + "createCollection=true";
    return Connection::QueryPrefix::Next;
  }
  return prefix;
}

Connection::QueryPrefix Document::httpMergeQuery(
    std::string& url, const Flags flgs, const Connection::QueryPrefix prefix) {
  if (flgs & Options::Opt_NoMerge) {
    url += prefix + "mergeObjects=false";
    return Connection::QueryPrefix::Next;
  }
  return prefix;
}

Connection::QueryPrefix Document::httpKeepNullQuery(
    std::string& url, const Flags flgs, const Connection::QueryPrefix prefix) {
  if (flgs & Options::Opt_RemoveNull) {
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
  Flags flgs = opts.flags();
  std::string url{pCol->docColUrl()};
  httpSyncQuery(url, flgs);
  httpCreateQuery(url, flgs);
  conn.reset();
  conn.setUrl(url);
  conn.setPostField(json);
  conn.setPostReq();
  conn.setBuffer();
  conn.setReady((flgs & Options::Opt_RunAsync) != 0);
}

//
// Configure to delete a document with the set key name in the
// Database/Collection
// location determined by the pCol parameter
//
void Document::httpDelete(const Collection::SPtr& pCol,
                          const Connection::SPtr& pCon, const Options& opts) {
  typedef Connection::QueryPrefix QueryPrefix;
  Connection& conn = *pCon;
  const Flags flgs = opts.flags();
  std::string url = pCol->refDocUrl(_key);
  QueryPrefix pre = httpSyncQuery(url, flgs, QueryPrefix::First);
  pre = httpRevQuery(url, opts, pre);
  httpPolicyQuery(url, flgs, pre);
  conn.reset();
  conn.setUrl(url);
  conn.setDeleteReq();
  conn.setBuffer();
  conn.setReady((flgs & Options::Opt_RunAsync) != 0);
}

Connection::QueryPrefix Document::httpRevQuery(
    std::string& url, const DocOptions& opts,
    const Connection::QueryPrefix prefix) {
  const std::string rev = opts.eTag();
  if (!rev.empty()) {
    const Flags match = opts.flags() & Options::Opt_MatchRev;
    if (match != 0) {
      url += prefix + "rev=" + rev;
      return Connection::QueryPrefix::Next;
    }
  }
  return prefix;
}

void Document::httpMatchHeader(Connection::HttpHeaderList& headers,
                               const Options& opts) {
  std::string rev = opts.eTag();
  if (!rev.empty()) {
    std::string opt;
    const Flags match = opts.flags() & Options::Opt_MatchMask;
    switch (match) {
      default: { return; }
      case Options::Opt_MatchRev: {
        opt += "If";
        break;
      }
      case Options::Opt_NoneMatchRev: {
        opt += "If-None";
        break;
      }
    }
    opt += "-Match:\"" + rev + "\"";
    headers.push_back(opt);
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
  Connection& conn = *pCon;
  const Flags flgs = opts.flags();
  Connection::HttpHeaderList headers;
  httpMatchHeader(headers, opts);
  conn.reset();
  conn.setUrl(pCol->refDocUrl(_key));
  conn.setGetReq();
  if (!headers.empty()) {
    conn.setHeaderOpts(headers);
  }
  conn.setBuffer();
  conn.setReady((flgs & Options::Opt_RunAsync) != 0);
}

//
// Replace a Document with new values in VelocyPack data
//
// All options implemented, match rev done as a query
//
void Document::httpReplace(const Collection::SPtr& pCol,
                           const Connection::SPtr& pCon, const Options& opts,
                           Connection::VPack data) {
  typedef Connection::QueryPrefix QueryPrefix;
  Connection& conn = *pCon;
  const Flags flgs = opts.flags();
  std::string url = pCol->refDocUrl(_key);
  QueryPrefix pre = httpSyncQuery(url, flgs, QueryPrefix::First);
  pre = httpPolicyQuery(url, flgs, pre);
  httpRevQuery(url, opts, pre);
  conn.reset();
  conn.setUrl(url);
  conn.setPostField(Connection::json(data, false));
  conn.setPutReq();
  conn.setBuffer();
  conn.setReady((flgs & Options::Opt_RunAsync) != 0);
}

//
// DONE
//
void Document::httpPatch(const Collection::SPtr& pCol,
                         const Connection::SPtr& pCon, const Options& opts,
                         Connection::VPack data) {
  typedef Connection::QueryPrefix QueryPrefix;
  Connection& conn = *pCon;
  Flags flgs = opts.flags();
  std::string url{pCol->refDocUrl(_key)};
  QueryPrefix pre = httpSyncQuery(url, flgs, QueryPrefix::First);
  pre = httpMergeQuery(url, flgs, pre);
  pre = httpRevQuery(url, opts, pre);
  pre = httpPolicyQuery(url, flgs, pre);
  httpKeepNullQuery(url, flgs, pre);
  conn.reset();
  conn.setPatchReq();
  conn.setUrl(url);
  conn.setPostField(Connection::json(data, false));
  conn.setBuffer();
  conn.setReady((flgs & Options::Opt_RunAsync) != 0);
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
    split = line.find_first_not_of("\t ", split + 1);
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
  Connection& conn = *pCon;
  const Flags flgs = opts.flags();
  std::string url = pCol->refDocUrl(_key);
  httpRevQuery(url, opts, Connection::QueryPrefix::First);
  conn.reset();
  conn.setHeadReq();
  conn.setUrl(url);
  conn.setBuffer();
  conn.setReady((flgs & Options::Opt_RunAsync) != 0);
}
}
}
