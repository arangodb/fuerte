# -*- mode: CMAKE; -*-

set(FuerteBenchSRCS
  main.cpp
  BucketReadTest.cpp
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
