add_executable(test_main
    test_main.cpp
    test_vst.cpp
    test_connection_basic_http.cpp
    test_connection_basic_vst.cpp
    test_10000_writes.cpp
)

target_link_libraries(test_main fuerte velocypack gtest)
add_test(new_all test_main)
