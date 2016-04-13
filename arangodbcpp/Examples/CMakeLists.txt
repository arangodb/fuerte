if (NOT CMAKE_PROJECT_NAME)

  cmake_minimum_required(VERSION 3.1)

  project(Examples)

  include("../../Settings.CMake")

  set(arangodbcppPath "${CMAKE_CURRENT_SOURCE_DIR}/..")

  find_library(arangodbcppLib NAMES arangodbcpp PATHS
  "${arangodbcppPath}/build/src"
  "${arangodbcppPath}/src"
  "${CMAKE_INSTALL_PREFIX}/lib")

  if (NOT arangodbcppLib)

      set(errorMessage "arangodbcpp library not built/installed")

  endif (NOT arangodbcppLib)

endif (NOT CMAKE_PROJECT_NAME)

if (errorMessage)

    message("${errorMessage}")

else (errorMessage)

    set(TestApps FuerteBench)

    foreach(app ${TestApps})

        add_subdirectory(${app})

    endforeach()

endif (errorMessage)
