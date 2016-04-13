if (CMAKE_PROJECT_NAME)

  set(FuerteBenchSRCS
    main.cpp
    BucketReadTest.cpp
    FuerteBench.cpp
    )

  set(FuerteBenchLIBS
    curl
    curlpp
    "${CMAKE_THREAD_LIBS_INIT}"
    arangodbcpp
    velocypack
    )

  add_executable(FuerteBench
    ${FuerteBenchSRCS}
  )

  target_link_libraries(FuerteBench
      ${FuerteBenchLIBS}
  )

endif (CMAKE_PROJECT_NAME)
 
