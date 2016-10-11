# -*- mode: CMAKE; -*-
cmake_minimum_required(VERSION 3.0)
project(fuerte CXX)

# CMAKE CXX STANDARD work since cmake 3.1
# FIXME
# we need the workaround until we require 3.1
# at the moment debian stable is at CMAKE 3.0
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(UNIX) #required by curlpp which is broken
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

#FIXME use set_target_property instead of messing with global flags!
#set_target_property(foo bar PROPERTY POSITION_INDEPENDENT_CODE ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
enable_testing()

#########################################################################################
# Configuration
#########################################################################################

option(FUERTE_TESTS     "Build Tests" OFF)
option(FUERTE_EXAMPLES "Build EXAMPLES" OFF)

#########################################################################################
# Dependencies
#########################################################################################

find_package(Boost COMPONENTS serialization system REQUIRED)
option(BuildVelocyPackExamples "Build examples" OFF)
add_subdirectory(driver/3rdParty/velocypack)
add_subdirectory(driver/3rdParty/curlpp2)

#########################################################################################
# Main Project
#########################################################################################


##  xxhash.cpp
add_library(fuerte STATIC
    driver/src/ConnectionUrl.cpp
    driver/src/HttpConnection.cpp
    driver/src/VppConnection.cpp
    driver/src/Connection.cpp
    driver/src/ConOption.cpp
    driver/src/Server.cpp
    driver/src/Database.cpp
    driver/src/Collection.cpp
    driver/src/Document.cpp
    driver/src/Cursor.cpp
    driver/src/Version.cpp
    driver/src/HeaderCommon.cpp
    driver/src/HeaderMulti.cpp
    driver/src/HeaderLoader.cpp
)

target_include_directories(fuerte PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/include
)
target_include_directories(fuerte PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/3rdParty/curlpp2/include
)

target_link_libraries(fuerte PUBLIC
    velocypack
    curlpp
    ${Boost_LIBRARIES}
)
target_compile_definitions(fuerte PUBLIC
    BOOST_ASIO_DISABLE_THREADS
)
#DVELOCYPACK_XXHASH=1

#########################################################################################
# Tests
#########################################################################################

if(FUERTE_TESTS)
#if(CMAKE_TESTING)
    add_subdirectory(driver/3rdParty/googletest)
    add_subdirectory(driver/tests)
endif()


#add_custom_target(run-tests)

#########################################################################################
# Examples
#########################################################################################
 
if(FUERTE_EXAMPLES)
    add_subdirectory(driver/examples)
endif()

#########################################################################################
# Install
#########################################################################################
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(UNIX)
        set(CMAKE_INSTALL_PREFIX  "$ENV{HOME}/local")
    else()
        message("not implemented for other operating systems")
    endif()
endif()

install(TARGETS fuerte DESTINATION lib)

install(
    DIRECTORY   "driver/include/fuerte"
    DESTINATION include
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#copy compile commands to source dir
if( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
    message(STATUS "copy compile_commands.json")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()

include(CPack)
