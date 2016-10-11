# basic
add_executable(BasicTests
    BasicTests/main.cpp
    BasicTests/TestApp.cpp
    BasicTests/SrvTest.cpp
    BasicTests/DbTest.cpp
)
target_link_libraries(BasicTests fuerte gtest)
add_test(basic BasicTests "http://127.0.0.1:8529" "*")
 
# aql
add_executable(AqlTests
    AqlTests/main.cpp
    AqlTests/TestApp.cpp
    AqlTests/AqlTest.cpp
)
target_link_libraries(AqlTests
    fuerte
    gtest
)
add_test(aql AqlTests "http://127.0.0.1:8529")

# col
add_executable(ColTests
    ColTests/main.cpp
    ColTests/TestApp.cpp
    ColTests/ColTest.cpp
)
target_link_libraries(ColTests fuerte gtest)
add_test(col ColTests "http://127.0.0.1:8529")

# doc
add_executable(DocTests
    DocTests/main.cpp
    DocTests/TestApp.cpp
    DocTests/DocTest.cpp
)
target_link_libraries(DocTests fuerte gtest)
add_test(doc DocTests "http://127.0.0.1:8529")

# dev
add_executable(DevTests DevTests/main.cpp DevTests/test.cpp)
target_link_libraries(DevTests fuerte)
