#include <curlpp/cURLpp.hpp>
#include <sstream>

#include "arangodbcpp/server.h"
#include "arangodbcpp/database.h"


namespace arangodb
{

namespace dbinterface
{

	uint16_t Server::m_inst = 0;
	
	/**

		Create a shared pointer to a server object

	*/
	Server::SPtr Server::create()
	{
		return SPtr(new Server());
	}

	Server::Server() : m_host{ "http://localhost:8529" }
	{
		if (!m_inst)
		{
			curlpp::initialize();
		}
		++m_inst;
	}
	
	Server::~Server()
	{
		--m_inst;
		if (!m_inst)
		{
			curlpp::terminate();
		}
	}

	/**

		Enables the user to set the host url

	*/
	void Server::setHostUrl(std::string url,uint16_t port)
	{
		std::ostringstream os;
		os << "http://" << url << ":" << port;
		m_host = os.str();
	}

	/**

		Configure to request the Arangodb version

	*/
	void Server::httpVersion(Connection::SPtr p,bool bAsync)
	{
		Connection &conn = *p;
		std::ostringstream os;
		os << m_host << "/_api/version";
		conn.reset();
		conn.setUrl( os.str() );
		conn.setBuffer();
		conn.setReady(bAsync);
	}
	
	/**

		Configure to request the current default Database

	*/
	void Server::httpCurrentDb(Connection::SPtr p,bool bAsync)
	{
		Connection &conn = *p;
		std::ostringstream os;
		os << m_host << "/_api/database/current";
		conn.reset();
		conn.setUrl( os.str() );
		conn.setBuffer();
		conn.setReady(bAsync);
	}
	
	/**

		Configure to request the user Databases available

	*/
	void Server::httpUserDbs(Connection::SPtr p,bool bAsync)
	{
		Connection &conn = *p;
		std::ostringstream os;
		os << m_host << "/_api/database/user";
		conn.reset();
		conn.setUrl( os.str() );
		conn.setBuffer();
		conn.setReady(bAsync);
	}
	
	/**

		Configure to request the Databases available

	*/
	void Server::httpExistingDbs(Connection::SPtr p,bool bAsync)
	{
		Connection &conn = *p;
		std::ostringstream os;
		os << m_host << "/_api/database";
		conn.reset();
		conn.setUrl( os.str() );
		conn.setBuffer();
		conn.setReady(bAsync);
	}
}
}
