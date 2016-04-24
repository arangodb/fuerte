# -*- mode: CMAKE; -*-

set(DocTestsSRCS
  main.cpp
  TestApp.cpp
  DocTest.cpp
)

add_executable(DocTests
  ${DocTestsSRCS}
)

target_link_libraries(DocTests
  ${arangodbcppLib}
  gtest
  ${velocypackLib}
  ${curlLib}
  ${LIBCURLPP_LIBS}
)
