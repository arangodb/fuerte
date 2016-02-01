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

#ifndef DATABASE_H
#define DATABASE_H

#include <memory>

#include "arangodbcpp/connection.h"

namespace arangodb
{
	
namespace dbinterface
{

class Server;


class Database
{
	public:
		typedef std::shared_ptr<Database> SPtr;
		Database() = delete;
		Database(std::shared_ptr<Server> srv, std::string name = "_system");
		~Database();
		void httpCreate(Connection::SPtr conn, bool bAsync = false);
		Connection::VPack httpCreate(bool bSort, Connection::SPtr conn);
		void httpDrop(Connection::SPtr conn, bool bAsync = false);
		Connection::VPack httpDrop(bool bSort, Connection::SPtr conn);
	private:
		std::shared_ptr<Server> m_server;
		std::string m_name;
};

inline Database::~Database()
{
}

inline Connection::VPack Database::httpCreate(bool bSort, Connection::SPtr conn)
{
	return conn->fromJSon(bSort);
}

inline Connection::VPack Database::httpDrop(bool bSort, Connection::SPtr conn)
{
	return conn->fromJSon(bSort);
}


}
}

#endif // DATABASE_H
