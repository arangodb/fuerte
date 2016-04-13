if (NOT DEFINED CMAKE_PROJECT_NAME)

  cmake_minimum_required(VERSION 3.1)

  project(ColTests)

  set(arangodbcppPath "${CMAKE_CURRENT_SOURCE_DIR}/../..")

  find_library(arangodbcppLib NAMES arangodbcpp PATHS
    "${arangodbcppPath}/build/src"
    "${arangodbcppPath}/src"
    "${CMAKE_INSTALL_PREFIX}/lib")

  if (NOT arangodbcppLib)

    set(errorMessage "arangodbcpp library not built")

  endif (NOT arangodbcppLib)

  if (NOT errorMessage)

    find_library(gtestcppLib NAMES gtest)

    if (NOT gtestcppLib)

      set(errorMessage "Google gtest library not installed")

    endif (NOT gtestcppLib)

  endif (NOT errorMessage)

  include("../../../Settings.CMake")

endif (NOT DEFINED CMAKE_PROJECT_NAME)

if (errorMessage)

  message("${errorMessage}")

else (errorMessage)

  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include" "${arangodbcppPath}/include")
  add_subdirectory(src)

endif (errorMessage)
