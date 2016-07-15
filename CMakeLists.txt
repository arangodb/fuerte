# -*- mode: CMAKE; -*-

cmake_minimum_required(VERSION 2.8)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release
    CACHE string
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE
  )
endif ()

if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"
      OR CMAKE_BUILD_TYPE STREQUAL "Release"
      OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"
      OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"
      OR CMAKE_BUILD_TYPE STREQUAL "None"))
  message(FATAL_ERROR "expecting CMAKE_BUILD_TYPE: None Debug Release RelWithDebInfo MinSizeRel, got ${CMAKE_BUILD_TYPE}.")
endif ()

project(fuerte CXX)

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
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CMAKE_COMPILER_IS_GNUCC 1)
endif ()

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

#########################################################################################
# Sub-Projects
#########################################################################################

add_subdirectory(3rdParty/googletest)
add_subdirectory(3rdParty/curlpp2)

#########################################################################################
# Main-Project
#########################################################################################

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
