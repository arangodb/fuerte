if (CMAKE_PROJECT_NAME)

  set(FuerteBenchSRCS
    main.cpp
    BucketReadTest.cpp
    FuerteBench.cpp
    )

  set(FuerteBenchLIBS
    ${arangodbcppLib}
    ${CMAKE_THREAD_LIBS_INIT}
    ${curlLib}
    ${LIBCURLPP_LIBS}
    ${velocypackLib}
    )

  add_executable(FuerteBench
    ${FuerteBenchSRCS}
  )

  target_link_libraries(FuerteBench
      ${FuerteBenchLIBS}
  )

endif (CMAKE_PROJECT_NAME)
 
