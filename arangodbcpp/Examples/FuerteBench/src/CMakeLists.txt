if (CMAKE_PROJECT_NAME)

  set(FuerteBenchSRCS
    main.cpp
    BucketReadTest.cpp
    FuerteBench.cpp
    )

  set(FuerteBenchLIBS
    ${arangodbcppLib}
    ${CMAKE_THREAD_LIBS_INIT}
    curl
    curlpp
    velocypack
    )

  add_executable(FuerteBench
    ${FuerteBenchSRCS}
  )

  target_link_libraries(FuerteBench
      ${FuerteBenchLIBS}
  )

endif (CMAKE_PROJECT_NAME)
 
