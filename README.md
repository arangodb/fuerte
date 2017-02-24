# fuerte

Fuerte is a c++ library that allows you to communicate with a ArangoDB database
over the http and velocystream (optionally ssl encrypted) protocols.

## design

Fuerte is a communication library only. You will get what the other side is
sending you. No conversion is done! When receiving a message fuerte provides
content type and payload. In case the payload is velocypack you can access the
slices with slices() when using the c++ driver. The node driver will always
provide the payload as it is. 

## driver: C++ Driver for ArangoDB

The project to create the fuerte library is located in the subdirectory
`cmake-cxx-driver`. It uses `cmake` as build system.

## Build requirements

The following development packages need to be installed

- C++ 11 compiler
- cmake 3.0
- cURL: https://github.com/curl/curl (needs to be found by FindCURL.cmake)

## nodejs: a low-level node.js driver

Install node and npm and execute

```
> cd cmake-node-driver
> npm install
```

## status of fuerte

Basic functionality of the c++ and node driver are implemented:

Things that are missing:

- agenda - the task is to get the nosql-tests working - what do those tests require?
- tests - without tests we never know the exact status (below is a list of missing featues)
- hanging with 100k requests (needs to be found)
- c++/node: incomplete handling of broken connections - need to find out what is missing (worse in node)
- c++: missing handling of endianess
- http/vst: no authentication
- http/vst: content type handling needs testing
- http: only first slice is added as payload
- vst: sending only single chunk messages
- vst: only the first slice is available via slices()
- vst: no compression
- vst: not handling all versions - velocystream version unknown (it works with the server)
- node: no good node integration (libuv)
- node: no real asynchronous work because of the above
- node: not building on different systems (locating of headers)

## License

Fuerte is published under the Apache 2 License. Please see
the files [LICENSE](LICENSE) and
[LICENSES-OTHER-COMPONENTS.md](LICENSES-OTHER-COMPONENTS.md)
for details.
