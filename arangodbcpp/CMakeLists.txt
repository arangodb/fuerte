set(BASE_COMPILER_OPTIONS "-std=c++11 -Wall -Wextra")
set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${BASE_COMPILER_OPTIONS}")

project(arangodbcpp)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)

add_subdirectory(src)
