# -*- mode: CMAKE; -*-

set(AqlTestsSRCS
  main.cpp
  TestApp.cpp
  AqlTest.cpp
)

add_executable(AqlTests
  ${AqlTestsSRCS}
)

target_link_libraries(AqlTests
  ${arangodbcppLib}
  gtest
  ${velocypackLib}
  ${curlLib}
  ${LIBCURLPP_LIBS}
)
