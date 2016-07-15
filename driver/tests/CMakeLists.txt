# -*- mode: CMAKE; -*-

include_directories( 
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/3rdParty/curlpp2/include
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/include
)

set(TestApps
  BasicTests
  ColTests
  DocTests
  AqlTests
)

foreach(app ${TestApps})
  add_subdirectory(${app})
  add_dependencies(run-tests run-${app})
  add_dependencies(run-${app} ${app})
endforeach()
