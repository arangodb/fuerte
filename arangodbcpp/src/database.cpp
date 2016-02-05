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

#include "arangodbcpp/server.h"
#include "arangodbcpp/database.h"
#include "arangodbcpp/collection.h"
#include "arangodbcpp/connection.h"

namespace arangodb
{
	
namespace dbinterface
{

Database::Database(Server::SPtr srv, std::string name) : m_server{srv},m_name(name)
{
}

/**

	Get the core database url

*/ 
std::string Database::getDatabaseUrl() const
{
	std::ostringstream os;
	os << m_server->getHostUrl();
	if (!m_name.empty())
	{
		os << "/_db/" << m_name;
	}
	return os.str();
}

/**

	Configure to create a Database using the configured name

*/
void Database::httpCreate(Connection::SPtr p, bool bAsync)
{
	std::ostringstream os;
	Connection &conn = *p;
	conn.reset();
	conn.setJsonContent();
	os << m_server->getHostUrl() << "/_api/database";
	conn.setUrl(os.str());
	os.str("");
	os << "{ \"name\":\"" << m_name << "\" }";
	conn.setPostReq();
	conn.setPostField(os.str());
	conn.setBuffer();
	conn.setReady(bAsync);
}

/**

	Configure to drop a Database using the configured name

*/
void Database::httpDelete(Connection::SPtr p, bool bAsync)
{
	std::ostringstream os;
	Connection &conn = *p;
	conn.reset();
	os << m_server->getHostUrl() << "/_api/database/" << m_name;
	conn.setUrl(os.str());
	conn.setDeleteReq();
	conn.setBuffer();
	conn.setReady(bAsync);
}

}
}