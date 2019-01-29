# Fuerte

Fuerte is a c++ library that allows you to communicate with a ArangoDB database
over the http and velocystream (optionally ssl encrypted) protocols.

## design

Fuerte is a communication library only. You will get what the other side is
sending you. No conversion is done! When receiving a message fuerte provides
content type and payload. In case the payload is velocypack you can access the
slices with slices() when using the c++ driver. The node driver will always
provide the payload as it is. 

Fuerte itself is designed to be mostly thread-safe. Creating and 
destroying the fuerte _Connection_ objects must be externally synchronized, 
otherwise the `fuerte::Connection` methods are thread-safe. 

Connections can handle multiple "producer" threads concurrently
sending requests. Of course it depends on the underlying
protocol implementation how "concurrent" requests really are. The velocystream 
protocol allows for full duplex communication, the HTTP only for half duplex
communication.

## Build requirements

The following development packages need to be installed

- C++ 11 compiler
- VelocyPack
- cmake 3.4
- Boost Libraries 1.67

VelocyPack source can be obtained from GitHub using the following git command into your chosen directory 
git clone https://github.com/arangodb/velocypack
Boost can be installed via the package manager of your choice

```bash
mkdir build; cd build
cmake .. \
      -DCMAKE_BUILD_TYPE=Debug \
      -DFUERTE_TESTS=On \
      -DFUERTE_EXAMPLES=ON \
      -DVELOCYPACK_SOURCE_DIR=../../velocypack
make -j4
```

To add for example the address sanitzer for _clang_ you can add `LDFLAGS="-fsanitize=address" CXXFLAGS="-fsanitize=address"`
in front of the _cmake_ command.

## Usage example

```c++
#include <fuerte/fuerte.h>
#include <velocystream/Slice.h>

int main(){
    using namespace arangodb::fuerte;
    EventLoopService eventLoopService;
    auto conn = ConnectionBuilder().host("vst://localhost:8529")
                                   .user("hund")
                                   .password("arfarf")
                                   .connect(eventLoopService);
    auto request = fu::createRequest(fu::RestVerb::Get, "/_api/version");
    auto result = _connection->sendRequest(std::move(request));
    auto slice = result->slices().front();
    std::cout << slice.get("version").copyString();
    std::cout << slice.get("server").copyString();
}
```

## Project status

Basic functionality of the c++ driver is implemented and used in ArangoDB >= 3.4

At the moment the [node driver](https://github.com/arangodb/node-arangodb-cxx) is not maintained.

## License

Fuerte is published under the Apache 2 License. Please see
the files [LICENSE](LICENSE) and
[LICENSES-OTHER-COMPONENTS.md](LICENSES-OTHER-COMPONENTS.md)
for details.
