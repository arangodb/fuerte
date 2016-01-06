set(BASE_COMPILER_OPTIONS "-std=c++11 -Wall -Wextra")
set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${BASE_COMPILER_OPTIONS}")

add_subdirectory("src")