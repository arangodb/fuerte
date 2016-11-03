cmake_minimum_required(VERSION 3.0)
project(arango-node-driver)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC") #curlpp has broken build system
add_subdirectory(arango-cxx-driver)

add_library(arango-node-driver SHARED
    Connection.cpp
    init.cpp
    Server.cpp
    Database.cpp
    Collection.cpp
)

# Gives our library file a .node extension without any "lib" prefix
set_target_properties(arango-node-driver PROPERTIES PREFIX "" SUFFIX ".node")

# Essential include files to build a node addon,
# You should add this line in every CMake.js based project
target_include_directories(arango-node-driver PRIVATE ${CMAKE_JS_INC})

# Essential library files to link to a node addon
# You should add this line in every CMake.js based project
target_link_libraries(arango-node-driver ${CMAKE_JS_LIB} fuerte)
