# -*- mode: CMAKE; -*-

project(fuerte)
cmake_minimum_required(VERSION 2.8)

# need c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_COMPILER_IS_CLANG 1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
  set(CMAKE_COMPILER_IS_CLANG 1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
  set(CMAKE_COMPILER_IS_INTEL 1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_COMPILER_IS_INTEL 1)
endif ()

if (CMAKE_COMPILER_IS_GNUCC OR APPLE)
  set(CXX_FLAGS "${CXX_FLAGS} -std=c++11")
endif ()

add_subdirectory(3rdParty/googletest)
add_subdirectory(3rdParty/curlpp2)

add_subdirectory(src)

add_subdirectory(tests)
add_subdirectory(examples)

install(FILES
  include/fuerte/Collection.h
  include/fuerte/ConOption.h
  include/fuerte/Connection.h
  include/fuerte/ConnectionUrl.h
  include/fuerte/Cursor.h
  include/fuerte/Database.h
  include/fuerte/DocOptions.h
  include/fuerte/Document.h
  include/fuerte/HttpConnection.h
  include/fuerte/Server.h
  include/fuerte/Version.h
  include/fuerte/fuerte_version.h
  DESTINATION include/fuerte
)

add_custom_target(prepare_node
  COMMAND
    make install DESTDIR=${PROJECT_SOURCE_DIR}/nodejs
)
