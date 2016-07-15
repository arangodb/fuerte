# -*- mode: CMAKE; -*-

set(ColTestsSRCS
  main.cpp
  TestApp.cpp
  ColTest.cpp
)

add_executable(ColTests
  ${ColTestsSRCS}
)

target_link_libraries(ColTests
  fuerte
  gtest
)
