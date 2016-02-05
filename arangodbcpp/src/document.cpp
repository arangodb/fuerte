/*
 * Copyright 2016 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <sstream>

#include "arangodbcpp/database.h"
#include "arangodbcpp/collection.h"
#include "arangodbcpp/document.h"

namespace arangodb
{

namespace dbinterface
{

Document::Document(std::string inp) : m_key(inp)
{

}

Document::~Document()
{

}

void Document::httpCreate(Collection::SPtr pCol,Connection::SPtr pCon,bool bAsync)
{
	Connection &conn = *pCon;
	std::ostringstream os;
	conn.reset();
	conn.setUrl( pCol->createDocUrl() );
	os << "{ \"_key\":\"" << m_key << "\" }";
	conn.setJsonContent();
	conn.setPostField(os.str());
	conn.setPostReq();
	conn.setBuffer();
	conn.setReady(bAsync);
}

void Document::httpDelete(Collection::SPtr pCol,Connection::SPtr pCon,bool bAsync)
{
	Connection &conn = *pCon;
	std::ostringstream os;
	conn.reset();
	conn.setUrl( pCol->refDocUrl(m_key) );
	conn.setDeleteReq();
	conn.setBuffer();
	conn.setReady(bAsync);
}

void Document::httpGet(Collection::SPtr pCol,Connection::SPtr pCon,bool bAsync)
{
	Connection &conn = *pCon;
	std::ostringstream os;
	conn.reset();
	conn.setUrl( pCol->refDocUrl(m_key) );
	conn.setBuffer();
	conn.setReady(bAsync);
}

}
}