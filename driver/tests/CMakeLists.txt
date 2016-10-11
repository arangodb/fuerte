add_executable(BasicTests
    BasicTests/main.cpp
    BasicTests/TestApp.cpp
    BasicTests/SrvTest.cpp
    BasicTests/DbTest.cpp
)
target_link_libraries(BasicTests fuerte gtest)
#add_custom_target(run-BasicTests COMMAND
#    ${CMAKE_CURRENT_BINARY_DIR}/BasicTests http://127.0.0.1:8529 '*'
#)

add_executable(AqlTests
    AqlTests/main.cpp
    AqlTests/TestApp.cpp
    AqlTests/AqlTest.cpp
)
target_link_libraries(AqlTests
    fuerte
    gtest
)
#add_custom_target(run-AqlTests COMMAND
#    ${CMAKE_CURRENT_BINARY_DIR}/AqlTests http://127.0.0.1:8529
#)


add_executable(ColTests
    ColTests/main.cpp
    ColTests/TestApp.cpp
    ColTests/ColTest.cpp
)
target_link_libraries(ColTests fuerte gtest)
#add_custom_target(run-${app} COMMAND
#    ${CMAKE_CURRENT_BINARY_DIR}/src/${app} http://127.0.0.1:8529
#)


add_executable(DocTests
    DocTests/main.cpp
    DocTests/TestApp.cpp
    DocTests/DocTest.cpp
)
target_link_libraries(DocTests fuerte gtest)
#add_custom_target(run-${app} COMMAND
#    ${CMAKE_CURRENT_BINARY_DIR}/src/${app} http://127.0.0.1:8529
#)


add_executable(DevTests DevTests/main.cpp DevTests/test.cpp)
target_link_libraries(DevTests fuerte)
