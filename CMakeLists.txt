# -*- mode: CMAKE; -*-

cmake_minimum_required(VERSION 2.8)

project(fuerte)

add_subdirectory(3rdParty/curlpp-cbffe73/curlpp)
add_subdirectory(3rdParty/googletest)

foreach (LINK_DIR ${LINK_DIRECTORIES})
  link_directories("${LINK_DIR}")
endforeach()

add_subdirectory(arangodbcpp)
