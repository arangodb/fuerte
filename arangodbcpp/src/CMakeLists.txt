set(arangodbcppSRCS
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
    ${curlppLib}
		)

add_library(
		arangodbcpp
		STATIC
		${arangodbcppSRCS}
)

install(TARGETS arangodbcpp DESTINATION lib)
