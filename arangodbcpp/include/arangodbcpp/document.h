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

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "arangodbcpp/connection.h"
#include "arangodbcpp/collection.h"

namespace arangodb
{

namespace dbinterface
{
	
	class Document
	{
		public:
			typedef std::shared_ptr<Document> SPtr;
			Document(std::string name = "NewDoc");
			~Document();
			void httpCreate(Collection::SPtr pCol,Connection::SPtr pCon,bool bAsync);
			void httpDelete(Collection::SPtr pCol,Connection::SPtr pCon,bool bAsync);
			void httpGet(Collection::SPtr pCol,Connection::SPtr pCon,bool bAsync);
			Connection::VPack httpCreate(bool bSort,Connection::SPtr pCon);
			Connection::VPack httpDelete(bool bSort,Connection::SPtr pCon);
			Connection::VPack httpGet(bool bSort,Connection::SPtr pCon);
		private:
			std::string m_key;
	};

	inline Connection::VPack Document::httpCreate(bool bSort,Connection::SPtr pCon)
	{
		return pCon->fromJSon(bSort);
	}

	inline Connection::VPack Document::httpDelete(bool bSort,Connection::SPtr pCon)
	{
		return pCon->fromJSon(bSort);
	}

	inline Connection::VPack Document::httpGet(bool bSort,Connection::SPtr pCon)
	{
		return pCon->fromJSon(bSort);
	}

}
}

#endif // DOCUMENT_H
