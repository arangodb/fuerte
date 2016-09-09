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

#include "../include/fuerte/Document.h"

#include <velocypack/Builder.h>

namespace arangodb {
namespace dbinterface {
Document::Document(const std::string& inp) : _key(inp) {}

Document::Document(std::string&& inp) : _key(inp) {}

Document::~Document() {}

void Document::addKeyAttrib(arangodb::velocypack::Builder& builder) {
  builder.add("_key", arangodb::velocypack::Value(_key));
}

void Document::syncQuery(Connection& conn, const Options& opts) {
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

void Document::mergeQuery(Connection& conn, const Options& opts) {
  if (opts.flagged(Options::Merge::No)) {
    conn.addQuery(ConOption("mergeObjects", "false"));
  }
}

void Document::keepNullQuery(Connection& conn, const Options& opts) {
  if (opts.flagged(Options::RemoveNull::Yes)) {
    conn.addQuery(ConOption{"KeepNull", "false"});
  }
}

// Configure to create a new empty document with the location determined by the
// pCol parameter
void Document::create(const Collection::SPtr& pCol,
                      const Connection::SPtr& pCon, const Options& opts) {
  Connection& conn = pCon->reset();
  std::string json{"{\"_key\":\"" + _key + "\"}"};
  { conn.setHeaderOpts(); }
  {
    syncQuery(conn, opts);
    conn.setUrl(pCol->refColUrl());
  }
  conn.setPostField(json);
  conn.setPostReq();
  conn.setBuffer();
}

void Document::create(const Collection::SPtr& pCol,
                      const Connection::SPtr& pCon,
                      const Connection::VPack data, const Options& opts) {
  Connection& conn = pCon->reset();
  { conn.setHeaderOpts(); }
  {
    syncQuery(conn, opts);
    conn.setUrl(pCol->refColUrl());
  }
  conn.setPostField(data);
  conn.setPostReq();
  conn.setBuffer();
}

// Configure to delete a document with the set key name in the
// Database/Collection
// location determined by the pCol parameter
void Document::remove(const Collection::SPtr& pCol,
                      const Connection::SPtr& pCon, const Options& opts) {
  Connection& conn = pCon->reset();
  {
    revMatch(conn, opts);
    conn.setHeaderOpts();
  }
  {
    syncQuery(conn, opts);
    conn.setUrl(pCol->refDocUrl(_key));
  }
  conn.setDeleteReq();
  conn.setBuffer();
}

void Document::revMatch(Connection& conn, const Options& opts) {
  const std::string& rev = opts;
  if (!rev.empty() && opts.flagged(Options::Rev::Match)) {
    ConOption opt{"If-Match", '"' + rev + '"'};
    conn.addHeader(opt);
  }
}

//  Ensures Match _rev headers are mutually exclusive and require
//  an ETag
void Document::matchHeader(Connection& conn, const Options& opts) {
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

// Configure to get a document with the set key name in the
// Database/Collection with options
//
// Location determined by the pCol parameter
void Document::get(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                   const Options& opts) {
  Connection& conn = pCon->reset();
  matchHeader(conn, opts);
  conn.setUrl(pCol->refDocUrl(_key));
  conn.setGetReq();
  conn.setHeaderOpts();
  conn.setBuffer();
}

// Replace a Document with new values in VelocyPack data
//
// All options implemented, match rev done as a query
void Document::replace(const Collection::SPtr& pCol,
                       const Connection::SPtr& pCon, Connection::VPack data,
                       const Options& opts) {
  Connection& conn = pCon->reset();
  {
    revMatch(conn, opts);
    conn.setHeaderOpts();
  }
  {
    syncQuery(conn, opts);
    conn.setUrl(pCol->refDocUrl(_key));
  }
  conn.setPostField(data);
  conn.setPutReq();
  conn.setBuffer();
}

void Document::patch(const Collection::SPtr& pCol, const Connection::SPtr& pCon,
                     Connection::VPack data, const Options& opts) {
  Connection& conn = pCon->reset();
  {
    revMatch(conn, opts);
    conn.setHeaderOpts();
  }
  {
    syncQuery(conn, opts);
    mergeQuery(conn, opts);
    keepNullQuery(conn, opts);
    conn.setUrl(pCol->refDocUrl(_key));
  }
  conn.setPatchReq();
  conn.setPostField(data);
  conn.setBuffer();
}
}
}
