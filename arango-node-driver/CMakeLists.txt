cmake_minimum_required(VERSION 3.0)
# Name of the project (will be the name of the plugin)
project(node-arangodb)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC") #curlpp has broken build system
add_subdirectory(arango-cxx-driver)

# Build a shared library named after the project from the files in `src/`
add_library(node-arangodb SHARED
    Connection.cpp
    init.cpp
    Server.cpp
)

# Gives our library file a .node extension without any "lib" prefix
set_target_properties(node-arangodb PROPERTIES PREFIX "" SUFFIX ".node")

# Essential include files to build a node addon,
# You should add this line in every CMake.js based project
target_include_directories(node-arangodb PRIVATE ${CMAKE_JS_INC})

# Essential library files to link to a node addon
# You should add this line in every CMake.js based project
target_link_libraries(node-arangodb ${CMAKE_JS_LIB} fuerte)
