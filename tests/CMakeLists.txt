add_executable(test_main
    test_main.cpp
    test_vst.cpp
    test_connection_basic.cpp
    test_10000_writes.cpp
)

target_link_libraries(test_main
    fuerte
    velocypack
    gtest
    Boost::system
    Boost::thread
    ${CMAKE_THREAD_LIBS_INIT}
)
add_test(new_all test_main)
