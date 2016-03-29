set(DocTestsSRCS
  main.cpp
  TestApp.cpp
  DocTest.cpp
	)

set(arangodbcppDir "${PROJECT_SOURCE_DIR}/../..")
#
# This needs to be more generic for target OS
#
set(arangodbcppLIB "${arangodbcppDir}/build/src/libarangodbcpp.a")

include_directories("${PROJECT_SOURCE_DIR}/include" "${arangodbcppDir}/include" )


set(DocTestsLIBS
        ${arangodbcppLIB}
        gtest
        velocypack
        curl
        curlpp
        )

add_executable(DocTests
  ${DocTestsSRCS}
)

target_link_libraries(DocTests
    ${DocTestsLIBS}
)
