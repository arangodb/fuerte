# fuerte
C++ Driver for ArangoDB

The project to create the fuerte library is located in the
subdirectory `driver`. The project uses `cmake`. In order to
build

```
> cd driver
> mkdir build
> cd build
> cmake ..
> make
```

The will create the fuerte library. You can install it using

```
> make install
```




Build requirements
-----------------------

The following development packages need to be installed

    cURL                   https://github.com/curl/curl
    VelocyPack             https://github.com/arangodb/velocypack
    
Building every thing
-------------------------

    mkdir build
    cd build
    cmake ..
    make

If no errors have occurred everything should now be built.
