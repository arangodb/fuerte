# -*- mode: CMAKE; -*-

add_definitions("-DVELOCYPACK_XXHASH=1")

#
# Assumes boost is installed
#
find_package(Boost
  COMPONENTS serialization system
  REQUIRED)

#
# Ensure threads are not required by Boost ASIO
#
add_definitions(-DBOOST_ASIO_DISABLE_THREADS )

include_directories( 
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/3rdParty/curlpp2/include
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/include
)

set(fuerteSRCS
  ConnectionUrl.cpp
  HttpConnection.cpp
  VppConnection.cpp
  Connection.cpp
  ConOption.cpp
  Server.cpp
  Database.cpp
  Collection.cpp
  Document.cpp
  Cursor.cpp
  Version.cpp
  HeaderCommon.cpp
  HeaderMulti.cpp
  HeaderLoader.cpp
)

set(velocypackSRCS
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/AttributeTranslator.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Builder.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Collection.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Dumper.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Exception.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/HexDump.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Iterator.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Options.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Parser.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Slice.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/ValueType.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Validator.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/Version.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/asm-functions.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/fpconv.cpp
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/src/velocypack-common.cpp
  xxhash.cpp
)

message("${Boost_LIBRARIES}")

add_library(fuerte
  STATIC
  ${fuerteSRCS}
  ${velocypackSRCS}
  $<TARGET_OBJECTS:curlpp_OBJS>
)

get_directory_property(CURL_LIBRARIES
  DIRECTORY ${PROJECT_SOURCE_DIR}/3rdParty/curlpp2
  DEFINITION CURL_LIBRARIES
)

target_link_libraries(fuerte PUBLIC
    ${CURL_LIBRARIES}   
    ${Boost_LIBRARIES}
)

install(TARGETS fuerte DESTINATION lib)
