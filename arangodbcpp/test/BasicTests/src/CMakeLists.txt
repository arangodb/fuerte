if (CMAKE_PROJECT_NAME)

  set(BasicTestsSRCS
    main.cpp
    TestApp.cpp
    SrvTest.cpp
    DbTest.cpp
    )

  set(BasicTestsLIBS
          arangodbcpp
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

endif (CMAKE_PROJECT_NAME)
