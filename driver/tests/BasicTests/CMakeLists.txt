# -*- mode: CMAKE; -*-

include_directories(include)
add_subdirectory(src)

add_custom_target(run-${app}
  COMMAND
    ${CMAKE_CURRENT_BINARY_DIR}/src/${app} http://127.0.0.1:8529 '*'
)
