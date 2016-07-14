# -*- mode: CMAKE; -*-

project(fuerte)
cmake_minimum_required(VERSION 2.8)

# need c++11
set(CMAKE_CXX_STANDARD 11)

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
