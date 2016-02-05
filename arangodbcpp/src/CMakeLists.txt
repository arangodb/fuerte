
set(arangodbcppSRCS
	connection.cpp
	server.cpp
	database.cpp
	collection.cpp
	document.cpp
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
