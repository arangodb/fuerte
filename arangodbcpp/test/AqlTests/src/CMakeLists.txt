if (CMAKE_PROJECT_NAME)

  set(AqlTestsSRCS
    main.cpp
    TestApp.cpp
    AqlTest.cpp
    )

  set(AqlTestsLIBS
          ${arangodbcppLib}
          gtest
          velocypack
          curl
          curlpp
          )

  add_executable(AqlTests
    ${AqlTestsSRCS}
  )

  target_link_libraries(AqlTests
      ${AqlTestsLIBS}
  )

endif (CMAKE_PROJECT_NAME)
