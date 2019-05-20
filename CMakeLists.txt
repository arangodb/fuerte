cmake_minimum_required(VERSION 3.4)
project(fuerte CXX C)

# Configuration
option(FUERTE_TESTS            "Build Tests" OFF)
option(FUERTE_EXAMPLES         "Build EXAMPLES" OFF)
option(FUERTE_STANDALONE_ASIO  "Use standalone ASIO" OFF)

message(STATUS "FUERTE_STANDALONE_ASIO ${FUERTE_STANDALONE_ASIO}")

enable_testing()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#########################################################################################
# Dependencies

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(Threads REQUIRED)

find_package(Boost REQUIRED COMPONENTS "system" "thread")

if(VELOCYPACK_SOURCE_DIR)
    option(BuildVelocyPackExamples "Build examples" OFF)
    add_subdirectory(${VELOCYPACK_SOURCE_DIR} ./vpack-build)
    set(VELOCYPACK_LIBRARIES velocypack)
    add_library(arangodb::velocypack ALIAS velocypack) # should be done in velocypack lib itself
else()
    include(FindVelocypack)
    find_package(Velocypack)
    if (NOT ${VELOCYPACK_FOUND})
        message(FATAL_ERROR "Velocypack not found. Clone from https://github.com/arangodb/velocypack and install it")
    endif()
endif()

#########################################################################################
# Main Project

## fuerte
add_library(fuerte STATIC
    src/connection.cpp
    src/ConnectionBuilder.cpp
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
    src/connection.cpp
    src/http_parser/http_parser.c
)

target_link_libraries(fuerte PUBLIC
    ${VELOCYPACK_LIBRARIES}
    Boost::system
    Boost::thread
    ${OPENSSL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)
target_include_directories(fuerte PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${VELOCYPACK_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    $<$<BOOL:${FUERTE_STANDALONE_ASIO}>:${ASIO_SOURCE_DIR}>
)

target_compile_definitions(fuerte PUBLIC
    $<$<CONFIG:Debug>:FUERTE_CHECKED_MODE>
    $<$<CONFIG:Debug>:FUERTE_DEBUG>
    $<$<BOOL:${FUERTE_STANDALONE_ASIO}>:FUERTE_STANDALONE_ASIO=1>
)

add_executable(fuerte-get tools/fuerte-get.cpp)
target_link_libraries(fuerte-get PUBLIC fuerte)

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
# Install
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(UNIX)
        set(CMAKE_INSTALL_PREFIX  "$ENV{HOME}/.local")
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
