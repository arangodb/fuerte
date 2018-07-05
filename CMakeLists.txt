cmake_minimum_required(VERSION 3.4)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

project(fuerte CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
    # -fPIC here as curlpp is borken
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

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

find_package(CURL REQUIRED)

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
# Main Project
# FIXME -FPIC does not end up in curlpp
#set_target_properties(fuerte-old velocypack curlpp PROPERTIES POSITION_INDEPENDENT_CODE ON)
#DVELOCYPACK_XXHASH=1

## fuerte
add_library(fuerte STATIC
    src/connection.cpp
    src/ConnectionBuilder.cpp
    src/CurlMultiAsio.cpp
    src/database.cpp
    src/helper.cpp
    src/HttpConnection.cpp
    src/loop.cpp
    src/message.cpp
    src/requests.cpp
    src/types.cpp
    src/vst.cpp
    src/VstConnection.cpp
    src/collection.cpp
    src/connection.cpp
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "Enabling Fuerte checked mode: -DFUERTE_CHECKED_MODE")
    target_compile_definitions(fuerte PUBLIC "FUERTE_CHECKED_MODE" )
endif()

target_link_libraries(fuerte PUBLIC
    ${VELOCYPACK_LIBRARIES}
    ${CURL_LIBRARIES}
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
