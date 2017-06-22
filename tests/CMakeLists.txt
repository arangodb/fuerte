# -------------------------------------
# Build test program
# -------------------------------------

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

# -------------------------------------
# Configure tests (general)
# -------------------------------------

set(TEST_DB_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/db")
file(REMOVE_RECURSE "${TEST_DB_FOLDER}")

set(TEST_JWT_FILE "${TEST_DB_FOLDER}/jwt.secret")
file(WRITE "${TEST_JWT_FILE}" "please-invent-a-better-secret")

find_program(ARANGODB "arangodb"
             HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../../arangodb")

# -------------------------------------
# Test: single server, no authentication
# -------------------------------------

add_test(NAME dbSingleNoAuth COMMAND test_main)

add_test(NAME startSingleServerNoAuth 
         COMMAND ${ARANGODB} start --starter.wait 
            --starter.local
            --starter.mode=single 
            --starter.data-dir=${TEST_DB_FOLDER}/single-no-auth)
add_test(NAME stopSingleServerNoAuth COMMAND ${ARANGODB} stop)

set_tests_properties(dbSingleNoAuth          PROPERTIES FIXTURES_REQUIRED DbNoAuth)
set_tests_properties(startSingleServerNoAuth PROPERTIES FIXTURES_SETUP DbNoAuth)
set_tests_properties(stopSingleServerNoAuth  PROPERTIES FIXTURES_CLEANUP DbNoAuth)

# -------------------------------------
# Test: single server, basic authentication
# -------------------------------------

add_test(NAME dbSingleBasicAuth COMMAND test_main)

add_test(NAME startSingleServerBasicAuth 
         COMMAND ${ARANGODB} start --starter.wait 
            --starter.local
            --starter.mode=single 
            --starter.data-dir=${TEST_DB_FOLDER}/single-basic-auth
            --auth.jwt-secret=${TEST_JWT_FILE})
add_test(NAME stopSingleServerBasicAuth COMMAND ${ARANGODB} stop)

set_tests_properties(dbSingleBasicAuth          PROPERTIES ENVIRONMENT "TEST_AUTHENTICATION=basic:root:")
set_tests_properties(dbSingleBasicAuth          PROPERTIES FIXTURES_REQUIRED DbBasicAuth)
set_tests_properties(startSingleServerBasicAuth PROPERTIES FIXTURES_SETUP DbBasicAuth)
set_tests_properties(stopSingleServerBasicAuth  PROPERTIES FIXTURES_CLEANUP DbBasicAuth)
