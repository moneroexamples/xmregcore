
macro(add_test_target _TEST_NAME)

    add_executable(${_TEST_NAME}_tests
            ${_TEST_NAME}_tests.cpp
            JsonTx.cpp)

    target_link_libraries(${_TEST_NAME}_tests
            PRIVATE
            myxrmcore
            gtest gtest_main
            gmock gmock_main
            ${LIBRARIES})

    add_test(NAME ${_TEST_NAME}_tests COMMAND ${_TEST_NAME}_tests)

endmacro()

resource_dir("res")

add_test_target(universalidentifier)
add_test_target(account)
add_test_target(tools)

