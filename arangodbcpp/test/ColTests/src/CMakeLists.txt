set(ColTestsSRCS
  main.cpp
  TestApp.cpp
  ColTest.cpp
	)

set(arangodbcppDir "${PROJECT_SOURCE_DIR}/../..")
#
# This needs to be more generic for target OS
#
set(arangodbcppLIB "${arangodbcppDir}/build/src/libarangodbcpp.a")

include_directories("${PROJECT_SOURCE_DIR}/include" "${arangodbcppDir}/include" )


set(ColTestsLIBS
        ${arangodbcppLIB}
        gtest
        velocypack
        curl
        curlpp
        )

add_executable(ColTests
	${ColTestsSRCS}
)

target_link_libraries(ColTests
		${ColTestsLIBS}
)
