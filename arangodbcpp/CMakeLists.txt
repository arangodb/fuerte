project(arangodbcpp)

cmake_minimum_required(VERSION 3.1)

if (NOT velocypackLib)

find_library(velocypackLib NAMES velocypack PATHS "${CMAKE_INSTALL_PREFIX}/bin")

endif (NOT velocypackLib)

if (velocypackLib)

if (NOT configBuild)

include("../Settings.CMake")

endif (NOT configBuild)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_INSTALL_PREFIX}/include")

add_subdirectory(src)
add_subdirectory(include)

else (velocypackLib)

message("velocypack library not installed")

endif (velocypackLib)
