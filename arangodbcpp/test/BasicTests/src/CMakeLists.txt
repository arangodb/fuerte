set(BasicTestsSRCS
	main.cpp
  TestApp.cpp
  SrvTest.cpp
  DbTest.cpp
	)

set(arangodbcppDir "${PROJECT_SOURCE_DIR}/../..")
#
# This needs to be more generic for target OS
#
set(arangodbcppLIB "${arangodbcppDir}/build/src/libarangodbcpp.a")

include_directories("${PROJECT_SOURCE_DIR}/include" "${arangodbcppDir}/include" )


set(BasicTestsLIBS
        ${arangodbcppLIB}
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
