# -*- mode: CMAKE; -*-

cmake_minimum_required(VERSION 3.0)

project(fuerte CXX)

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

# need c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
    #required by curlpp
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

#set_target_property(foo bar PROPERTY POSITION_INDEPENDENT_CODE ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

find_package(Boost
  COMPONENTS serialization system
  REQUIRED)


#########################################################################################
# Sub-Projects
#########################################################################################
set(BuildVelocyPackExamples OFF)
add_subdirectory(driver/3rdParty/velocypack)
add_subdirectory(driver/3rdParty/googletest)
add_subdirectory(driver/3rdParty/curlpp2)

#########################################################################################
# Main Project
#########################################################################################



add_definitions("-DVELOCYPACK_XXHASH=1")
add_definitions(-DBOOST_ASIO_DISABLE_THREADS )

include_directories( 
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/include
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/3rdParty/curlpp2/include
)
    #${CMAKE_CURRENT_SOURCE_DIR}/driver/3rdParty/velocypack/include

set(fuerte_src
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

##set(velocypackSRCS
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/AttributeTranslator.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Builder.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Collection.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Dumper.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Exception.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/HexDump.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Iterator.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Options.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Parser.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Slice.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/ValueType.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Validator.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Version.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/asm-functions.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/fpconv.cpp
##  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/velocypack-common.cpp
##  xxhash.cpp
##)

add_library(fuerte
  STATIC
  ${fuerte_src}
)

target_include_directories(fuerte PUBLIC
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(fuerte PUBLIC
    velocypack
    curlpp
    ${Boost_LIBRARIES}
)

# install(TARGETS fuerte DESTINATION lib)
# 
# 
# 
# add_subdirectory(src)
# 
# install(FILES
#   include/fuerte/Collection.h
#   include/fuerte/ConOption.h
#   include/fuerte/Connection.h
#   include/fuerte/ConnectionUrl.h
#   include/fuerte/Cursor.h
#   include/fuerte/Database.h
#   include/fuerte/DocOptions.h
#   include/fuerte/Document.h
#   include/fuerte/HttpConnection.h
#   include/fuerte/Server.h
#   include/fuerte/Version.h
#   include/fuerte/fuerte_version.h
#   DESTINATION include/fuerte
# )
# 
# #########################################################################################
# # Tests
# #########################################################################################
# 
# add_custom_target(run-tests)
# add_subdirectory(tests)
# 
# #########################################################################################
# # Examples
# #########################################################################################
# 
# add_subdirectory(examples)
