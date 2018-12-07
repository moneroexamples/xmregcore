
macro(add_om_test _TEST_NAME)

    add_executable(${_TEST_NAME}_tests
            ${_TEST_NAME}_tests.cpp
            JsonTx.cpp)

    target_include_monero_directories(${_TEST_NAME}_tests)

    target_link_libraries(${_TEST_NAME}_tests
            PRIVATE
            gtest gtest_main
            gmock gmock_main
            ${LIBRARIES})

    add_test(NAME ${_TEST_NAME}_tests COMMAND ${_TEST_NAME}_tests)

endmacro()

resource_dir("res")


add_om_test(universalidentifier)
add_om_test(account)
