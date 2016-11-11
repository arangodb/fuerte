cmake_minimum_required(VERSION 3.0)

project(fuerte CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

# FIXME
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
# workaround pre 3.1 - waiting for debian stable
if(UNIX)
    # -fPIC here as curlpp is borken
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
endif()

#########################################################################################
# Configuration
option(FUERTE_TESTS    "Build Tests" OFF)
option(FUERTE_EXAMPLES "Build EXAMPLES" OFF)

#########################################################################################
# Dependencies
find_package(Boost COMPONENTS serialization system REQUIRED)
option(BuildVelocyPackExamples "Build examples" OFF)
add_subdirectory(3rdParty/velocypack)
add_subdirectory(3rdParty/curlpp2)

#########################################################################################
# Main Project
add_library(fuerte-old STATIC
    src/old/ConnectionUrl.cpp
    src/old/HttpConnection.cpp
    src/old/VppConnection.cpp
    src/old/Connection.cpp
    src/old/ConOption.cpp
    src/old/Server.cpp
    src/old/Database.cpp
    src/old/Collection.cpp
    src/old/Document.cpp
    src/old/Cursor.cpp
    src/old/Version.cpp
    src/old/HeaderCommon.cpp
    src/old/HeaderMulti.cpp
    src/old/HeaderLoader.cpp
)
#  xxhash.cpp

target_include_directories(fuerte-old PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/curlpp2/include
)

# FIXME
#target_include_directories(fuerte PRIVATE
#    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/curlpp2/include
#)

target_link_libraries(fuerte-old PUBLIC
    velocypack
    curlpp
    ${Boost_LIBRARIES}
)
target_compile_definitions(fuerte-old PUBLIC
    BOOST_ASIO_DISABLE_THREADS
)

# FIXME -FPIC does not end up in curlpp
set_target_properties(fuerte-old velocypack curlpp PROPERTIES POSITION_INDEPENDENT_CODE ON)
#DVELOCYPACK_XXHASH=1

## fuerte-next
add_library(fuerte STATIC
    src/database.cpp
    src/collection.cpp
    src/request.cpp
    src/connection.cpp
    src/vst.cpp
    src/server.cpp
    src/asio.cpp
)

target_link_libraries(fuerte PUBLIC
    velocypack
    curlpp
    ${Boost_LIBRARIES}
)

target_include_directories(fuerte PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/curlpp2/include
)

add_subdirectory(3rdParty/googletest) # should not be required by examples
#########################################################################################
# Tests
if(FUERTE_TESTS)
#if(CMAKE_TESTING)
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
