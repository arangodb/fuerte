set(DocTestsSRCS
  main.cpp
  TestApp.cpp
  DocTest.cpp
	)

set(DocTestsLIBS
        ${arangodbcppLib}
        gtest
        ${velocypackLib}
        curl
        curlpp
        )

add_executable(DocTests
  ${DocTestsSRCS}
)

target_link_libraries(DocTests
    ${DocTestsLIBS}
)

if (configBuild)

add_dependencies(DocTests arangodbcpp)

endif (configBuild)
