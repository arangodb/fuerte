cmake_minimum_required(VERSION 3.4)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

project(fuerte CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#########################################################################################
# Configuration
option(FUERTE_TESTS    "Build Tests" OFF)
option(FUERTE_EXAMPLES "Build EXAMPLES" OFF)

#########################################################################################
# Dependencies

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(Threads REQUIRED)

set(CMAKE_BOOST_COMPONENTS "system" "thread")
find_package(Boost REQUIRED COMPONENTS ${CMAKE_BOOST_COMPONENTS})

if(VELOCYPACK_SOURCE_DIR)
    option(BuildVelocyPackExamples "Build examples" OFF)
    add_subdirectory(${VELOCYPACK_SOURCE_DIR} ./vpack-build)
    set(VELOCYPACK_LIBRARIES velocypack)
else()
    include(FindVelocypack)
    find_package(Velocypack)

    if (NOT ${VELOCYPACK_FOUND})
        message(FATAL_ERROR "Velocypack not found. Clone from https://github.com/arangodb/velocypack and install it")
    endif()
endif()

#########################################################################################
# compiler options
if (CMAKE_COMPILER_IS_GNUCC)
  if (VERBOSE)
    message(STATUS "Compiler type GNU: ${CMAKE_CXX_COMPILER}")
  endif ()

  set(BASE_FLAGS "-Wall -Wextra -Wno-unused-parameter ${BASE_FLAGS}")

  set(CMAKE_C_FLAGS                "-g"                             CACHE INTERNAL "default C compiler flags")
  set(CMAKE_C_FLAGS_DEBUG          "-O0 -g -D_DEBUG=1"              CACHE INTERNAL "C debug flags")
  set(CMAKE_C_FLAGS_MINSIZEREL     "-Os"                            CACHE INTERNAL "C minimal size flags")
  set(CMAKE_C_FLAGS_RELEASE        "-O3 -fomit-frame-pointer"       CACHE INTERNAL "C release flags")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -fno-omit-frame-pointer" CACHE INTERNAL "C release with debug info flags")

  set(CMAKE_CXX_FLAGS                "-g -Wnon-virtual-dtor"          CACHE INTERNAL "default C++ compiler flags")
  set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -D_DEBUG=1"              CACHE INTERNAL "C++ debug flags")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os"                            CACHE INTERNAL "C++ minimal size flags")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -fomit-frame-pointer"       CACHE INTERNAL "C++ release flags")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -fno-omit-frame-pointer" CACHE INTERNAL "C++ release with debug info flags")

elseif (CMAKE_COMPILER_IS_CLANG)
  if (VERBOSE)
    message(STATUS "Compiler type CLANG: ${CMAKE_CXX_COMPILER}")
  endif ()

  set(BASE_FLAGS "-Wall -Wextra -Wno-unused-parameter ${BASE_FLAGS}")

  set(CMAKE_C_FLAGS                "-g"                             CACHE INTERNAL "default C compiler flags")
  set(CMAKE_C_FLAGS_DEBUG          "-O0 -g -D_DEBUG=1"              CACHE INTERNAL "C debug flags")
  set(CMAKE_C_FLAGS_MINSIZEREL     "-Os"                            CACHE INTERNAL "C minimal size flags")
  set(CMAKE_C_FLAGS_RELEASE        "-O3 -fomit-frame-pointer"       CACHE INTERNAL "C release flags")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -fno-omit-frame-pointer" CACHE INTERNAL "C release with debug info flags")

  set(CMAKE_CXX_FLAGS                "-g -Wnon-virtual-dtor"          CACHE INTERNAL "default C++ compiler flags")
  set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -D_DEBUG=1"              CACHE INTERNAL "C++ debug flags")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os"                            CACHE INTERNAL "C++ minimal size flags")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -fomit-frame-pointer"       CACHE INTERNAL "C++ release flags")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -fno-omit-frame-pointer" CACHE INTERNAL "C++ release with debug info flags")

elseif (MSVC)
  if (VERBOSE)
    message(STATUS "Compiler type MSVC: ${CMAKE_CXX_COMPILER}")
  endif ()

  set(CMAKE_C_FLAGS                "/MTd"                              CACHE INTERNAL "default C++ compiler flags")
  set(CMAKE_C_FLAGS_DEBUG          "/D _DEBUG /MTd /Zi /Ob0 /Od /RTC1 /bigobj" CACHE INTERNAL "C++ debug flags")
  set(CMAKE_C_FLAGS_MINSIZEREL     "/MT /O1 /Ob1"                      CACHE INTERNAL "C++ minimal size flags")
  set(CMAKE_C_FLAGS_RELEASE        "/MT /O2 /Ob2"                      CACHE INTERNAL "C++ release flags")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "/MT /Zi /O2 /Ob1"                  CACHE INTERNAL "C++ release with debug info flags")

  set(CMAKE_CXX_FLAGS                "/MTd"                              CACHE INTERNAL "default C++ compiler flags")
  set(CMAKE_CXX_FLAGS_DEBUG          "/D _DEBUG /MTd /Zi /Ob0 /Od /RTC1 /bigobj" CACHE INTERNAL "C++ debug flags")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "/MT /O1 /Ob1"                      CACHE INTERNAL "C++ minimal size flags")
  set(CMAKE_CXX_FLAGS_RELEASE        "/MT /O2 /Ob2"                      CACHE INTERNAL "C++ release flags")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT /Zi /O2 /Ob1"                  CACHE INTERNAL "C++ release with debug info flags")

else ()
  # unknown compiler
  message(STATUS "Compiler type UNKNOWN: ${CMAKE_CXX_COMPILER}")

  set(BASE_FLAGS "-Wall ${BASE_FLAGS}")

  set(CMAKE_C_FLAGS                "-g"                             CACHE INTERNAL "default C compiler flags")
  set(CMAKE_C_FLAGS_DEBUG          "-O0 -g"                         CACHE INTERNAL "C debug flags")
  set(CMAKE_C_FLAGS_MINSIZEREL     "-Os"                            CACHE INTERNAL "C minimal size flags")
  set(CMAKE_C_FLAGS_RELEASE        "-O3 -fomit-frame-pointer"       CACHE INTERNAL "C release flags")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -fno-omit-frame-pointer" CACHE INTERNAL "C release with debug info flags")

  set(CMAKE_CXX_FLAGS                "-g"                             CACHE INTERNAL "default C++ compiler flags")
  set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g"                         CACHE INTERNAL "C++ debug flags")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os"                            CACHE INTERNAL "C++ minimal size flags")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -fomit-frame-pointer"       CACHE INTERNAL "C++ release flags")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -fno-omit-frame-pointer" CACHE INTERNAL "C++ release with debug info flags")
endif ()

#########################################################################################
# Main Project
# FIXME -FPIC does not end up in curlpp
#set_target_properties(fuerte-old velocypack curlpp PROPERTIES POSITION_INDEPENDENT_CODE ON)
#DVELOCYPACK_XXHASH=1

## fuerte
add_library(fuerte STATIC
    src/connection.cpp
    src/ConnectionBuilder.cpp
#    src/database.cpp
    src/helper.cpp
    src/http.cpp
    src/HttpConnection.cpp
    src/jwt.cpp
    src/loop.cpp
    src/message.cpp
    src/requests.cpp
    src/types.cpp
    src/vst.cpp
    src/VstConnection.cpp
    src/AsioConnection.cpp
#    src/collection.cpp
    src/connection.cpp
    src/http_parser/http_parser.c
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "Enabling Fuerte checked mode: -DFUERTE_CHECKED_MODE")
    target_compile_definitions(fuerte PUBLIC "FUERTE_CHECKED_MODE" )
endif()

target_link_libraries(fuerte PUBLIC
    ${VELOCYPACK_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_include_directories(fuerte PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

add_executable(fuerte-get
    tools/fuerte-get.cpp
)

target_link_libraries(fuerte-get PRIVATE
    fuerte
)

#add_executable(vst-echo-server
#    tools/vst-echo-server.cpp
#)
#
#target_link_libraries(vst-echo-server PRIVATE
#    fuerte
#)

#########################################################################################
# Tests
if(FUERTE_TESTS)
    add_subdirectory(3rdParty/googletest)
    add_subdirectory(tests)
endif()

#########################################################################################
# Examples
if(FUERTE_EXAMPLES)
    add_subdirectory(examples)
endif()

#########################################################################################
# Clang Completion
#copy compile commands to source dir
if( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
    message(STATUS "copy compile_commands.json")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()


#########################################################################################
# Install
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(UNIX)
        set(CMAKE_INSTALL_PREFIX  "$ENV{HOME}/local")
    else()
        message("not implemented for other operating systems")
    endif()
endif()

install(TARGETS fuerte DESTINATION lib)

install(
    DIRECTORY   "include/fuerte"
    DESTINATION include
)

include(CPack) #should be last command
