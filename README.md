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


## License

Fuerte is published under the Apache 2 License. Please see
the files [LICENSE](LICENSE) and
[LICENSES-OTHER-COMPONENTS.md](LICENSES-OTHER-COMPONENTS.md)
for details.
