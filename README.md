# fuerte

## driver: C++ Driver for ArangoDB

The project to create the fuerte library is located in the subdirectory
`cmake-cxx-driver`. It uses `cmake` as build system. In order to build

```
> cd ./build
```

The will create the fuerte library. You can install it using

```
> make install
```

If you start an ArangoDB server on 127.0.0.1:8259, you can run
some tests

```
> cd <build directory> && ctest
```

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

## License

Fuerte is published under the Apache 2 License. Please see
the files [LICENSE](LICENSE) and
[LICENSES-OTHER-COMPONENTS.md](LICENSES-OTHER-COMPONENTS.md)
for details.
