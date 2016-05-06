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

#include "TestApp.h"

#include <iostream>

template <unsigned type>
class RefSPtr {
 public:
  enum class Type1 : uint8_t { Val1, Val2, Val3, Val4, Val5 };
  enum class Type2 : uint8_t { Val1, Val2, Val3, Val4, Val5 };
  struct alignas(1) Opts {
    uint8_t v1 : 3;
    uint8_t v2 : 3;
  };
  typedef arangodb::dbinterface::Connection Connection;
  RefSPtr(const Connection::SPtr& inp) : _ptr(inp) {}
  RefSPtr() = delete;
  Connection* operator->() const { return _ptr.operator->(); }
  Connection& operator*() const { return *_ptr; }

 private:
  const Connection::SPtr& _ptr;
};

typedef RefSPtr<1> SPtrOut;
typedef RefSPtr<2> SPtrIn;

void Info1(SPtrOut&& inp) { std::cout << "Ver 1 : " << &*inp << std::endl; }

void Info2(SPtrIn&& inp) { std::cout << "Ver 2 : " << &*inp << std::endl; }

int main(const int argc, char* argv[]) {
  typedef SPtrOut::Connection Connection;
  Connection::SPtr pConn = std::make_shared<Connection>();
  Info1(pConn);
  Info2(pConn);
  std::cout << "Opts size : " << sizeof(SPtrIn::Opts) << std::endl;
  TestApp app{argc, argv};
  return app.run();
}
