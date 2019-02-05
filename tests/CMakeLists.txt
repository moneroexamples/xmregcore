
macro(add_test_target _TEST_NAME)

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

macro(add_cov_target _TEST_NAME)

    SETUP_TARGET_FOR_COVERAGE_GCOVR_HTML(
        NAME ${_TEST_NAME}_cov
        EXECUTABLE ${_TEST_NAME}_tests
        DEPENDENCIES ${_TEST_NAME}_tests)

endmacro()

resource_dir("res")


add_test_target(universalidentifier)
add_test_target(account)
add_test_target(tools)

add_cov_target(universalidentifier)
add_cov_target(account)
add_cov_target(tools)


