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
#include <sstream>
#include <velocypack/Builder.h>

#include "arangodbcpp/Database.h"
#include "arangodbcpp/Collection.h"
#include "arangodbcpp/Document.h"

namespace arangodb
{

namespace dbinterface
{

Document::Document(std::string inp) : _key(inp) {}

Document::~Document() {}

//
// Configure to create a new empty document with the set key name at the
// Database/Collection
// location determined by the pCol parameter
//
void Document::httpCreate(Collection::SPtr pCol, Connection::SPtr pCon,
						  bool bAsync)
{
	Connection &conn = *pCon;
	std::ostringstream os;
	conn.reset();
	conn.setUrl(pCol->createDocUrl());
	os << "{ \"_key\":\"" << _key << "\" }";
	conn.setJsonContent();
	conn.setPostField(os.str());
	conn.setPostReq();
	conn.setBuffer();
	conn.setReady(bAsync);
}

//
//	Configure to delete a document with the set key name in the
//	Database/Collection
//	location determined by the pCol parameter
//
void Document::httpDelete(Collection::SPtr pCol, Connection::SPtr pCon,
						  bool bAsync)
{
	Connection &conn = *pCon;
	std::ostringstream os;
	conn.reset();
	conn.setUrl(pCol->refDocUrl(_key));
	conn.setDeleteReq();
	conn.setBuffer();
	conn.setReady(bAsync);
}

//
//  Configure to get a document with the set key name in the
//		Database/Collection
//
//		location determined by the pCol parameter
//
void Document::httpGet(Collection::SPtr pCol, Connection::SPtr pCon,
					   bool bAsync)
{
	Connection &conn = *pCon;
	std::ostringstream os;
	conn.reset();
	conn.setUrl(pCol->refDocUrl(_key));
	conn.setBuffer();
	conn.setReady(bAsync);
}

void Document::httpPatch(Collection::SPtr pCol, Connection::SPtr pCon,
						 bool bAsync, Connection::VPack data)
{
	Connection &conn = *pCon;
	std::ostringstream os;
	conn.reset();
	conn.setPatchReq();
	conn.setUrl(pCol->refDocUrl(_key));
	conn.setPostField(Connection::json(data, false));
	conn.setBuffer();
	conn.setReady(bAsync);
}

Connection::VPack Document::httpHead(bool bSort, Connection::SPtr pCon)
{
  namespace VTest=arangodb::velocypack;
  using VTest::ValueType;
  using VTest::Value;
  using VTest::Options;
  using VTest::Builder;
  typedef std::string::size_type sz_type;
  auto processLine = [](const std::string line,Builder &build) -> void
  {
    sz_type split = line.find_first_of(':');
    if (split == std::string::npos)
    {
      return;
    }
    std::string name = line.substr(0,split);
    split = line.find_first_not_of("\t ",split+1);
    std::string value = line.substr(split,line.length()- split);
    if (std::isdigit(value[0]))
    {
      sz_type num = std::stoul(value);
      build.add(name,Value(num));
      return;
    }
    if (value[0] == '"')
    {
      split = value.find_first_of('"',1);
      value = value.substr(1,split-1);
    }
    build.add(name,Value(value));
  };
  Options opts;
  opts.sortAttributeNames = bSort;
  Builder build{ &opts };
  const std::string delimEol { "\r\n" };
  std::string res = pCon->bufString();
  sz_type old = 0;
  sz_type pos = res.find_first_of(delimEol,old);
  build.add(Value(ValueType::Object));  // Start building an object
  while (pos != std::string::npos)
  {
    if (pos != old)
    {
      processLine(res.substr(old,pos - old),build);
    }
    old = pos + 1;
    pos = res.find_first_of(delimEol,old);
  }
  pos = res.length();
  if (old != pos)
  {
    processLine(res.substr(old,pos - old),build);
  }
  build.close();
  return build.steal();
}

void Document::httpHead(Collection::SPtr pCol, Connection::SPtr pCon,
						bool bAsync)
{
	Connection &conn = *pCon;
	std::ostringstream os;
	conn.reset();
	conn.setHeadReq();
	conn.setUrl(pCol->refDocUrl(_key));
	conn.setBuffer();
	conn.setReady(bAsync);
}
}
}
