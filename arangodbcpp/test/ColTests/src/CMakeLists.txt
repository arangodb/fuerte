if (CMAKE_PROJECT_NAME)

  set(ColTestsSRCS
    main.cpp
    TestApp.cpp
    ColTest.cpp
    )

  set(ColTestsLIBS
    ${arangodbcppLib}
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

endif (CMAKE_PROJECT_NAME)
