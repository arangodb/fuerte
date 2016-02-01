#ifndef SERVER_H

#define SERVER_H

#include <memory>
#include <string.h>

#include "arangodbcpp/connection.h"

namespace arangodb
{

namespace dbinterface
{

	class Database;

	class Server
	{
		public:
			typedef std::shared_ptr<Server> SPtr;
			~Server();
			void httpVersion(Connection::SPtr conn,bool bAsync);
			Connection::VPack httpVersion(bool bSort,Connection::SPtr conn);
			void httpCurrentDb(Connection::SPtr conn
				,bool bAsync);
			Connection::VPack httpCurrentDb(bool bSort,Connection::SPtr conn);
			void httpUserDbs(Connection::SPtr conn,bool bAsync);
			Connection::VPack httpUserDbs(bool bSort,Connection::SPtr conn);
			void httpExistingDbs(Connection::SPtr conn,bool bAsync);
			Connection::VPack httpExistingDbs(bool bSort,Connection::SPtr conn);
			void setHost(std::string url,uint16_t port);
			const std::string &getHost() const;
			
			static SPtr create();
			std::shared_ptr<Database> createDatabase(std::string name = "_system");
		private:
			explicit Server();

			static uint16_t m_inst;
			std::string m_host;
	};
	
	inline Connection::VPack Server::httpVersion(bool bSort,Connection::SPtr conn)
	{
		return conn->fromJSon(bSort);
	}

	inline Connection::VPack Server::httpUserDbs(bool bSort,Connection::SPtr conn)
	{
		return conn->fromJSon(bSort);
	}

	inline Connection::VPack Server::httpCurrentDb(bool bSort,Connection::SPtr conn)
	{
		return conn->fromJSon(bSort);
	}

	inline Connection::VPack Server::httpExistingDbs(bool bSort,Connection::SPtr conn)
	{
		return conn->fromJSon(bSort);
	}


	inline const std::string &Server::getHost() const
	{
		return m_host;
	}

}
}

#endif // SERVER_H
