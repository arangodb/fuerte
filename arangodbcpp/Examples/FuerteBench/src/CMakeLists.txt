# -*- mode: CMAKE; -*-

set(FuerteBenchSRCS
  main.cpp
  BucketReadTest.cpp
  BucketWriteTest.cpp
  BucketTest.cpp
  FuerteBench.cpp
)

add_executable(FuerteBench
  ${FuerteBenchSRCS}
)

target_link_libraries(FuerteBench
  ${arangodbcppLib}
  gtest
  ${velocypackLib}
  ${curlLib}
  ${LIBCURLPP_LIBS}
)
