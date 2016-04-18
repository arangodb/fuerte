if (NOT CMAKE_PROJECT_NAME)

  cmake_minimum_required(VERSION 3.3)

  project(FuerteBench)

  include("../../../Settings.CMake")

  set(arangodbcppPath "${CMAKE_CURRENT_SOURCE_DIR}/../..")

  find_library(arangodbcppLib NAMES arangodbcpp PATHS
    "${arangodbcppPath}/../build/arangodbcpp/src"
    "${arangodbcppPath}/../arangodbcpp/src"
    "${arangodbcppPath}/build/src"
    "${arangodbcppPath}/src"
    "${CMAKE_INSTALL_PREFIX}/bin")

  if (NOT arangodbcppLib)

    set(ErrorMessage "arangodbcpp library not built/installed")

  endif (NOT arangodbcppLib)

  include_directories(
    "${arangodbcppPath}/include"
    "${CMAKE_INSTALL_PREFIX}/include")

endif (NOT CMAKE_PROJECT_NAME)

if (NOT ErrorMessage)

  find_package(Threads)

endif (NOT ErrorMessage)

#
# Check if error has occurred
#
if (ErrorMessage)

  message("${ErrorMessage}")

else (ErrorMessage)

  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
  add_subdirectory(src)

endif (ErrorMessage)
