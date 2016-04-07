# -*- mode: CMAKE; -*-

cmake_minimum_required(VERSION 3.1)

project(fuerte)

find_library(velocypackLib NAMES velocypack PATHS "${CMAKE_INSTALL_PREFIX}/bin")

if (velocypackLib)

include("Settings.CMake" RESULT_VARIABLE configBuild)

# Create library name
set(arangodbcppLib "${CMAKE_STATIC_LIBRARY_PREFIX}arangodbcpp${CMAKE_STATIC_LIBRARY_SUFFIX}")
# Create path to where it will be built, (linked from)
set(arangodbcppLib "${CMAKE_CURRENT_SOURCE_DIR}/arangodbcpp/src/${arangodbcppLib}")

add_subdirectory(arangodbcpp)
add_subdirectory(arangodbcpp/test)

else (velocypackLib)

message("velocypack library not installed")

endif (velocypackLib)
