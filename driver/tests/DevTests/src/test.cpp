#include <fuerte/Document.h>
////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author John Bufton
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author John Bufton
////////////////////////////////////////////////////////////////////////////////
#include <iostream>

#include "../include/test.h"

#define _VELOCYSTREAM_

namespace local {
#ifdef _VELOCYSTREAM_
const std::string url{"vstream://127.0.0.1:8529"};
#else
const std::string url{"http://127.0.0.1:8529"};
#endif
}

typedef arangodb::dbinterface::Server DbServer;
typedef arangodb::dbinterface::Database DbItem;
typedef arangodb::dbinterface::Document DbDoc;
typedef arangodb::dbinterface::Collection DbCollection;
typedef arangodb::dbinterface::Connection DbConnection;
typedef DbConnection::VPack DbVPack;

void displayResult(DbConnection::SPtr pConn) {
  auto resCode = pConn->responseCode();
  DbVPack resData = pConn->result();
  std::cout << "Response code : " << resCode << std::endl;
  std::cout << "Response data,  (JSon) : " << DbConnection::json(resData)
            << std::endl;
}

void test1() {
  DbServer::SPtr pSrv{new DbServer{local::url}};
  DbConnection::SPtr pConn{pSrv->makeConnection()};
  pSrv->version(pConn);
  pConn->run();
  displayResult(pConn);
  pSrv->version(pConn);
  pConn->setAsynchronous(true);
  do {
    pConn->run();
  } while (pConn->isRunning());
  displayResult(pConn);
}

void test2() {
  const std::string newName{"RenamedCol"};
  DbServer::SPtr pSrv{std::make_shared<DbServer>(local::url)};
  DbConnection::SPtr pConn{pSrv->makeConnection()};
  DbItem::SPtr pDb{std::make_shared<DbItem>(pSrv, std::string{"Test"})};
  pDb->create(pConn);
  pConn->run();
  displayResult(pConn);

  DbCollection::SPtr pCol{
      std::make_shared<DbCollection>(pDb, std::string{"MyTest"})};
  pCol->create(pConn);
  pConn->run();
  displayResult(pConn);

  DbDoc::SPtr pDoc{std::make_shared<DbDoc>(std::string{"MyDoc"})};
  // DbDoc::SPtr pDoc { new DbDoc{ std::string{"MyDoc"} } };
  pDoc->create(pCol, pConn, DbDoc::Options{});
  pConn->run();
  // displayResult(pConn);

  pCol->rename(pConn, newName);
  pConn->run();
  displayResult(pConn);
  *pCol = newName;
  pCol->remove(pConn);

  pConn->run();
  displayResult(pConn);
  pDb->remove(pConn);
  pConn->run();
  displayResult(pConn);
}
