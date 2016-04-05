project(ColTests)

cmake_minimum_required(VERSION 2.6)

set(ARANGODBCPP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include" "${ARANGODBCPP_PATH}/include" )

find_library(arangodbcppLib NAMES arangodbcpp PATHS "${ARANGODBCPP_PATH}/build/src" "${ARANGODBCPP_PATH}/src")

#
# Need to make sure these options work with other than gcc compiler
#
set(BASE_COMPILER_OPTIONS "-std=c++11 -Wall -Wextra")
set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${BASE_COMPILER_OPTIONS}")

add_subdirectory("src")
