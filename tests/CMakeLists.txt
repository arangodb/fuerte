add_executable(test_main
    test_main.cpp
    test_vst.cpp
    test_connection_basic.cpp
    test_connection_failures.cpp
    test_10000_writes.cpp
)

target_link_libraries(test_main
    fuerte
    gtest
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)
add_test(new_all test_main)
