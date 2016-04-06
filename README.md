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

Because the test programs require the arangodbcpp library this requires 4 steps

1 Create make file for  the arangodbcpp library
2 Build the arangodbcpp library
3 Create make file for  the arangodbcpp library and test programs
4 Build the arangodbcpp library and test programs

from  the home directory run the following commands

    mkdir build
    cd build
    cmake ..
    make

After the library has been build, you can generate the tests programs by
rerunning cmake / make

    cmake ..
    make

If no errors have occurred everything should now be built.
