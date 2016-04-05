
set(BasicTestsSRCS
	main.cpp
  TestApp.cpp
  SrvTest.cpp
  DbTest.cpp
	)

set(BasicTestsLIBS
        ${arangodbcppLib}
        gtest
        velocypack
        curl
        curlpp
        )

add_executable(BasicTests
	${BasicTestsSRCS}
)

target_link_libraries(BasicTests
		${BasicTestsLIBS}
)
