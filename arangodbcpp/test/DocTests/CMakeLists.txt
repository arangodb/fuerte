project(DocTests)

cmake_minimum_required(VERSION 3.1)

if (NOT velocypackLib)

find_library(velocypackLib NAMES velocypack PATHS "${CMAKE_INSTALL_PREFIX}/bin")

endif (NOT velocypackLib)

if (velocypackLib)

if (NOT configBuild)

include("../../../Settings.CMake")

endif (NOT configBuild)

set(ARANGODBCPP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include" "${ARANGODBCPP_PATH}/include" "${CMAKE_INSTALL_PREFIX}/include")

if (NOT arangodbcppLib)

find_library(arangodbcppLib NAMES arangodbcpp PATHS "${ARANGODBCPP_PATH}/build/src" "${ARANGODBCPP_PATH}/src" "${CMAKE_INSTALL_PREFIX}/lib")

endif (NOT arangodbcppLib)

if (arangodbcppLib)

add_subdirectory(src)

else (arangodbcppLib)

message("arangodbcpp library not built/installed")

endif (arangodbcppLib)

else (velocypackLib)

message("velocypack library not installed")

endif (velocypackLib)
