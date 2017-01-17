add_executable(test_main
    test_main.cpp
)

target_link_libraries(test_main fuerte velocypack gtest)
add_test(new_all test_main)
