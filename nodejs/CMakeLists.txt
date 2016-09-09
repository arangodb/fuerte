cmake_minimum_required(VERSION 3.0)

# Name of the project (will be the name of the plugin)
project(node-arangodb)

set(CPP_SRCS init.cpp Connection.cpp Server.cpp)
set(HPP_SRCS init.h Connection.h Server.h)

# Build a shared library named after the project from the files in `src/`
file(GLOB SOURCE_FILES ${CPP_SRCS} ${HPP_SRCS})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

# Gives our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Essential include files to build a node addon,
# You should add this line in every CMake.js based project
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/Modules")
add_subdirectory(driver)

include_directories(
  ${PROJECT_SOURCE_DIR}/driver/include
  ${PROJECT_SOURCE_DIR}/driver/3rdParty/curlpp2/include
  ${PROJECT_SOURCE_DIR}/driver/3rdParty/velocypack/include
)

# Essential library files to link to a node addon
# You should add this line in every CMake.js based project
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} fuerte)
