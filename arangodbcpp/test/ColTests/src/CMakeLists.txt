set(ColTestsSRCS
  main.cpp
  TestApp.cpp
  ColTest.cpp
	)

set(ColTestsLIBS
        ${arangodbcppLib}
        gtest
        ${velocypackLib}
        curl
        curlpp
        )

add_executable(ColTests
	${ColTestsSRCS}
)

target_link_libraries(ColTests
		${ColTestsLIBS}
)

if (configBuild)

add_dependencies(ColTests arangodbcpp)

endif (configBuild)
