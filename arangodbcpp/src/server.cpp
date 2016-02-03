#include <curlpp/cURLpp.hpp>
#include <sstream>

#include "arangodbcpp/server.h"
#include "arangodbcpp/database.h"


namespace arangodb
{

namespace dbinterface
{

	uint16_t Server::m_inst = 0;
	
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

	void Server::setHost(std::string url,uint16_t port)
	{
		std::ostringstream os;
		os << "http://" << url << ":" << port;
		m_host = os.str();
	}

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
