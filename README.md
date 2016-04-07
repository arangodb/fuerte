# fuerte
Low Level C++ Driver for ArangoDB

The project to create the arangodbcpp library, the project is located in the following subdirectory

    ~/arangodbcpp

With some test programs under this subdirectory

    ~/arangodbcpp/tests

Build requirements
-----------------------

The following development packages need to be installed

    cURL                   https://github.com/curl/curl
    cURLpp                 https://github.com/jpbarrette/curlpp
    GTEST                  https://github.com/google/googletest/tree/master/googletest
    VelocyPack             https://github.com/arangodb/velocypack
    
Building every thing
-------------------------

    mkdir build
    cd build
    cmake ..
    make

If no errors have occurred everything should now be built.
