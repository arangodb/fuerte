# #new tests
# add_executable(test_main
#     test_main.cpp
#     BasicTests.cpp
# )
# #target_include_directories(test_main PRIVATE
# #    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/velocypack/include
# #    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/googletest/googletest/include
# #)
# target_link_libraries(test_main fuerte velocypack gtest)
# add_test(new_test test_main)

# The following test are disabled
# aql
add_executable(AqlTests
    AqlTests/main.cpp
    AqlTests/TestApp.cpp
    AqlTests/AqlTest.cpp
)
target_link_libraries(AqlTests
    fuerte-old
    gtest
)
add_test(aql AqlTests "http://127.0.0.1:8529")

# col
add_executable(ColTests
    ColTests/main.cpp
    ColTests/TestApp.cpp
    ColTests/ColTest.cpp
)
target_link_libraries(ColTests fuerte-old gtest)
add_test(col ColTests "http://127.0.0.1:8529")

# doc
add_executable(DocTests
    DocTests/main.cpp
    DocTests/TestApp.cpp
    DocTests/DocTest.cpp
)
target_link_libraries(DocTests fuerte-old gtest)
add_test(doc DocTests "http://127.0.0.1:8529")

# dev
add_executable(DevTests DevTests/main.cpp DevTests/test.cpp)
target_link_libraries(DevTests fuerte-old)
