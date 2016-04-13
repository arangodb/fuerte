if (NOT CMAKE_PROJECT_NAME)

  cmake_minimum_required(VERSION 3.1)

  project(test)

  include("../../Settings.CMake")

endif (NOT CMAKE_PROJECT_NAME)

set(arangodbcppPath "${CMAKE_CURRENT_SOURCE_DIR}/..")

find_library(arangodbcppLib NAMES arangodbcpp PATHS
  "${arangodbcppPath}/build/src"
  "${arangodbcppPath}/src"
  "${CMAKE_INSTALL_PREFIX}/lib")

if (NOT arangodbcppLib)

  set(errorMessage "arangodbcpp library not built/installed")

endif (NOT arangodbcppLib)

if (NOT errorMessage)

  find_library(gtestcppLib NAMES gtest)

  if (NOT gtestcppLib)

    set(errorMessage "Google gtest library not installed")

  endif (NOT gtestcppLib)

endif (NOT errorMessage)

if (errorMessage)

  message("${errorMessage}")

else (errorMessage)

  set(TestApps BasicTests ColTests DocTests)

  foreach(app ${TestApps})

    add_subdirectory(${app})

  endforeach()

endif (errorMessage)
