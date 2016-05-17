# -*- mode: CMAKE; -*-

set(arangodbcppSRCS
  ConnectionUrl.cpp
  Connection.cpp
  Server.cpp
  Database.cpp
  Collection.cpp
  Document.cpp
  DocOptions.cpp
  Cursor.cpp
  Version.cpp
)

set(arangodbcppLIBS
  ${velocypackLib}
  ${curlLib}
  ${LIBCURLPP_LIBS}
)

add_library(arangodbcpp
  STATIC
  ${arangodbcppSRCS}
)

install(TARGETS arangodbcpp DESTINATION lib)
