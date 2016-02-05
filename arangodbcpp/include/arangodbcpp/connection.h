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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <memory>
#include <vector>
#include <list>
#include <velocypack/Buffer.h>
#include <curlpp/Easy.hpp>
#include <curlpp/Multi.hpp>
#include <curlpp/Options.hpp>

namespace arangodb
{
	
namespace dbinterface
{

/**

	Provide network connection to Arango database
	
	Hide the underlying use of cURL for implementation

*/
class Connection
{
	public:
		typedef std::shared_ptr<Connection> SPtr;
		typedef arangodb::velocypack::Buffer<uint8_t> VBuffer;
		typedef std::shared_ptr<VBuffer> VPack;
		typedef std::list<std::string> HeaderList;
		typedef char ErrBuf[CURL_ERROR_SIZE];
		Connection();
		~Connection();
		void setUrl(const std::string &inp);
		void setErrBuf(char *inp);
		void setPostField(const std::string &inp);
		void setJsonContent();
		void setHeaderOpts(HeaderList &inp);
		void setCustomReq(const std::string inp);
		void setPostReq();
		void setDeleteReq();
		void setVerbose(bool inp);
		void reset();
		template <typename T>
		void setBuffer(T *p,size_t (T::*f)(char *p,size_t sz,size_t m) );
		void setBuffer(size_t (*f)(char *p,size_t sz,size_t m) );
		void setBuffer();

		const std::string getBufString() const;
		VPack fromJSon(bool bSorted = true) const;
		static std::string toJson(VPack &v,bool bSort);
		void setReady(bool bAsync = false);
		void doRun();
		bool isError() const;
		bool isRunning() const;
		bool bufEmpty() const;
		
	private:
		enum Flags { 
			F_Multi = 1
			, F_Running = 2
			, F_Done = 4
			, F_LogicError = 8
			, F_RunError = 16
		};
		typedef std::vector<char> ChrBuf;
		size_t WriteMemoryCallback( char *ptr, size_t size, size_t nmemb );
		template <typename T>
		void setOpt(T inp);
		void asyncDo();
		void syncDo();
		void errFound(const std::string &inp,bool bRun = true);

		curlpp::Easy m_request;
		curlpp::Multi m_async;
		ChrBuf m_buf;
		uint8_t m_flgs;
};

template <typename T>
inline void Connection::setBuffer(T *p,size_t (T::*f)(char *,size_t,size_t) )
{
	curlpp::types::WriteFunctionFunctor functor(p,f);
	setOpt(curlpp::options::WriteFunction(functor));
}

inline bool Connection::bufEmpty() const
{
	return m_buf.empty();
}

inline void Connection::setPostReq()
{
	setCustomReq("POST");
}

inline void Connection::setDeleteReq()
{
	setCustomReq("DELETE");
}

inline void Connection::setErrBuf(char *inp)
{
	setOpt(cURLpp::options::ErrorBuffer(inp));
}

inline void Connection::setCustomReq(const std::string inp)
{
	setOpt(cURLpp::options::CustomRequest(inp));
}

inline void Connection::setVerbose(bool inp)
{
	setOpt(cURLpp::options::Verbose(inp));
}

inline bool Connection::isError() const
{
	if (m_flgs & (F_LogicError | F_RunError))
	{
		return true;
	}
	return false;
}

inline bool Connection::isRunning() const
{
	if (m_flgs & F_Running)
	{
		return true;
	}
	return false;
}


inline void Connection::setHeaderOpts(HeaderList &inp)
{
	setOpt(curlpp::options::HttpHeader(inp));
}

inline void Connection::setUrl(const std::string &inp)
{
	setOpt(curlpp::options::Url(inp) );
}

template <typename T>
inline void Connection::setOpt(T inp)
{
	m_request.setOpt(inp);
}

}
}

#endif // CONNECTION_H
