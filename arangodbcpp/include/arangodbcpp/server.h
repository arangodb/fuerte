#ifndef SERVER_H

#define SERVER_H

#include <memory>
#include <string.h>

#include "arangodbcpp/connection.h"

namespace arangodb
{

namespace dbinterface
{

	class Server
	{
		public:
			typedef std::shared_ptr<Server> SPtr;
			explicit Server();
			~Server();
			void httpCurrentDb(Connection &conn,bool bAsync);
			Connection::VPack httpCurrentDb(bool bSort,Connection &conn);
			void httpUserDbs(Connection &conn,bool bAsync);
			Connection::VPack httpUserDbs(bool bSort,Connection &conn);
			void httpExistingDbs(Connection &conn,bool bAsync);
			Connection::VPack httpExistingDbs(bool bSort,Connection &conn);
			void setHost(std::string url,uint16_t port);
		private:
			static uint16_t m_inst;
			std::string m_host;
	};
	
	inline Connection::VPack Server::httpUserDbs(bool bSort,Connection &conn)
	{
		return conn.fromJSon(bSort);
	}

	inline Connection::VPack Server::httpCurrentDb(bool bSort,Connection &conn)
	{
		return conn.fromJSon(bSort);
	}

	inline Connection::VPack Server::httpExistingDbs(bool bSort,Connection &conn)
	{
		return conn.fromJSon(bSort);
	}

}
}

#endif // SERVER_H
