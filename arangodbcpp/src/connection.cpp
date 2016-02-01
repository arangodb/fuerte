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
#include <cstring>
#include <velocypack/Slice.h>
#include <velocypack/Parser.h>
#include <velocypack/Builder.h>

#include "arangodbcpp/connection.h"

namespace arangodb
{
	
namespace dbinterface
{

void Connection::setBuffer()
{
	curlpp::types::WriteFunctionFunctor functor(this
		,&Connection::WriteMemoryCallback);
	setOpt(curlpp::options::WriteFunction(functor));
}

void Connection::setReady(bool bAsync)
{
	m_buf.clear();
	m_flgs = 0;
	if (bAsync)
	{
		m_flgs = F_Multi;
		m_async.add(&m_request);
	}
}

void Connection::reset()
{
	if (m_flgs & F_Multi)
	{
		m_async.remove(&m_request);
	}
	m_request.reset();
	m_flgs = 0;
}

void Connection::errFound(const std::string &inp,bool bRun)
{
	if (m_flgs & F_Multi)
	{
		m_async.remove(&m_request);
	}
	m_buf.clear();
	m_buf.insert(m_buf.begin(),inp.cbegin(),inp.cend());
	m_flgs = bRun?F_RunError:F_LogicError;
}

void Connection::syncDo()
{
	try
	{
		m_request.perform();
		m_flgs = F_Done;
	}
	catch ( curlpp::LogicError & e ) 
	{
		errFound(e.what(),false);
		return;
  }

  catch ( curlpp::LibcurlRuntimeError &e)
	{
		errFound(e.what());
		return;
	}
  
  catch ( curlpp::RuntimeError & e ) 
	{
		errFound(e.what());
		return;
  }
}

void Connection::asyncDo()
{
	try
	{
		{
			int nLeft;
			m_flgs |= F_Running;
			if (!m_async.perform(&nLeft))
			{
				return;
			}
			if (!nLeft)
			{
				m_async.remove(&m_request);
				m_flgs = F_Done;
				return;
			}
		}
		{
			struct timeval timeout;
			int rc; /* select() return code */

			fd_set fdread;
			fd_set fdwrite;
			fd_set fdexcep;
			int maxfd;

			FD_ZERO ( &fdread );
			FD_ZERO ( &fdwrite );
			FD_ZERO ( &fdexcep );

			/* set a suitable timeout to play around with */
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			
			/* get file descriptors from the transfers */
			m_async.fdset ( &fdread, &fdwrite, &fdexcep, &maxfd );

			rc = select ( maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout );
			if (rc == -1)
			{
				m_async.remove(&m_request);
				errFound("Select error");
				return;
			}
		}
	}
	catch ( curlpp::LogicError & e ) 
	{
		errFound(e.what(),false);
		return;
  }
  
  catch ( curlpp::LibcurlRuntimeError &e)
	{
		errFound(e.what());
		return;
	}
  
  catch ( curlpp::RuntimeError & e ) 
	{
		errFound(e.what());
		return;
  }
}

void Connection::setPostField(const std::string &inp)
{
	setOpt(curlpp::options::PostFields(inp));
	setOpt(curlpp::options::PostFieldSize(inp.length()));
}


size_t Connection::WriteMemoryCallback( char *ptr, size_t size, size_t nmemb )
{
	size_t realsize = size * nmemb;
	
	if (realsize != 0)
	{
		size_t offset = m_buf.size();
		m_buf.resize(offset + realsize);
		memcpy(&m_buf[offset],ptr,realsize);
	}

	return realsize;
}

const std::string Connection::getBufString() const
{
	std::string tmp;
	tmp.insert(tmp.begin(),m_buf.cbegin(),m_buf.cend());
	return tmp;
}

Connection::VPack Connection::fromJSon(bool bSorted) const
{
	if (!m_buf.empty())
	{
		using arangodb::velocypack::Builder;
		using arangodb::velocypack::Parser;
		using arangodb::velocypack::Options;
		Options options;
		options.sortAttributeNames = bSorted;
		Parser parser { &options };
		parser.parse(&m_buf[0],m_buf.size());
		std::shared_ptr<Builder> vp { parser.steal() };
		return vp->buffer();
	}
	return VPack { new VBuffer() };
}

Connection::Connection()
{
}

Connection::~Connection()
{
}

}
}