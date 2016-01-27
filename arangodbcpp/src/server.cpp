#include <curlpp/cURLpp.hpp>
#include <sstream>

#include "arangodbcpp/server.h"

namespace arangodb
{

namespace dbinterface
{

	uint16_t Server::m_inst = 0;

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

	void Server::httpCurrentDb(Connection &conn,bool bAsync)
	{
		std::ostringstream os;
		os << m_host << "/_api/database/current";
		conn.reset();
		conn.setUrl( os.str() );
		conn.setReady(bAsync);
	}
	

	void Server::httpUserDbs(Connection &conn,bool bAsync)
	{
		std::ostringstream os;
		os << m_host << "/_api/database/user";
		conn.reset();
		conn.setUrl( os.str() );
		conn.setReady(bAsync);
	}
	
	void Server::httpExistingDbs(Connection &conn,bool bAsync)
	{
		std::ostringstream os;
		os << m_host << "/_api/database";
		conn.reset();
		conn.setUrl( os.str() );
		conn.setReady(bAsync);
	}
}
}
