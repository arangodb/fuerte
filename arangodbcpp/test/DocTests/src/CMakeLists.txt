if (CMAKE_PROJECT_NAME)

  set(DocTestsSRCS
    main.cpp
    TestApp.cpp
    DocTest.cpp
    )

  set(DocTestsLIBS
    ${arangodbcppLib}
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

endif (CMAKE_PROJECT_NAME)
