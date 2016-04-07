
set(BasicTestsSRCS
	main.cpp
  TestApp.cpp
  SrvTest.cpp
  DbTest.cpp
	)

set(BasicTestsLIBS
        ${arangodbcppLib}
        gtest
        ${velocypackLib}
        curl
        curlpp
        )

add_executable(BasicTests
	${BasicTestsSRCS}
)

target_link_libraries(BasicTests
		${BasicTestsLIBS}
)

if (configBuild)

add_dependencies(BasicTests arangodbcpp)

endif (configBuild)
