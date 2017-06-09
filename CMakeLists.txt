cmake_minimum_required(VERSION 3.4)

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
find_package(Boost COMPONENTS ${CMAKE_BOOST_COMPONENTS})
if(Boost_FOUND)
    message(STATUS "== Using Boost provided by your system")
else()
    add_subdirectory(3rdParty/cmake-for-boost)
endif()

option(BuildVelocyPackExamples "Build examples" OFF)
add_subdirectory(3rdParty/velocypack)
add_subdirectory(3rdParty/curlpp2)

#########################################################################################
# Main Project
# FIXME -FPIC does not end up in curlpp
#set_target_properties(fuerte-old velocypack curlpp PROPERTIES POSITION_INDEPENDENT_CODE ON)
#DVELOCYPACK_XXHASH=1

## fuerte
add_library(fuerte STATIC
    src/types.cpp
    src/message.cpp
    src/vst.cpp
    src/loop.cpp
    src/helper.cpp
    src/HttpCommunicator.cpp
    src/HttpConnection.cpp
    src/VstConnection.cpp
    src/collection.cpp
    src/connection.cpp
    src/database.cpp
    src/requests.cpp
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "Enabling Fuerte checked mode: -DFUERTE_CHECKED_MODE")
    target_compile_definitions(fuerte PUBLIC "FUERTE_CHECKED_MODE" )
endif()

target_link_libraries(fuerte PUBLIC
    velocypack
    curlpp
    Boost::system
    Boost::thread
    ${OPENSSL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_include_directories(fuerte PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/curlpp2/include
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
