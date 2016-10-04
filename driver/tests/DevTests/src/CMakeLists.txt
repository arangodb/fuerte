# -*- mode: CMAKE; -*-

set(DevTestsSRCS
  main.cpp
  test.cpp
)

add_executable(DevTests
  ${DevTestsSRCS}
)

target_link_libraries(DevTests
  fuerte
)
 
