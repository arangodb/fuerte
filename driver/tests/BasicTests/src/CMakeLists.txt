# -*- mode: CMAKE; -*-

set(BasicTestsSRCS
  main.cpp
  TestApp.cpp
  SrvTest.cpp
  DbTest.cpp
)

add_executable(BasicTests
  ${BasicTestsSRCS}
)

target_link_libraries(BasicTests
  fuerte
  gtest
)
