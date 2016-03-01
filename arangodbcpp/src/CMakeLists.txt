set(arangodbcppSRCS
	Connection.cpp
	Server.cpp
	Database.cpp
	Collection.cpp
	Document.cpp
  DocOptions.cpp
  Cursor.cpp
	)

set(arangodbcppLIBS
		velocypack
		curl
		curlpp
		)

add_library(
		arangodbcpp
		STATIC
		${arangodbcppSRCS}
)

target_link_libraries(
		arangodbcpp
		${arangodbcppLIBS}
)
