# -*- mode: CMAKE; -*-

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/3rdParty/velocypack/include
)

set(FuerteBenchSRCS
  src/main.cpp
  src/BucketReadTest.cpp
  src/BucketTest.cpp
  src/BucketVersionTest.cpp
  src/BucketWriteTest.cpp
  src/FuerteBench.cpp
)

add_executable(FuerteBench
  ${FuerteBenchSRCS}
)

target_link_libraries(FuerteBench
  fuerte
  gtest
)
