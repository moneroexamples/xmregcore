cmake_minimum_required(VERSION 3.8)

set(PROJECT_NAME xmregcore_example)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_LIST_DIR}/cmake")

option(BUILD_XMREGCORE_TESTS 
    "Build tests for the project" ON)

include(MyUtils)

find_package(Monero)

add_executable(${PROJECT_NAME}
        example.cpp)

add_subdirectory(src/)

target_include_directories(myxrmcore
    PUBLIC  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)

target_link_libraries(${PROJECT_NAME}
    PRIVATE XMREG::core)

if (BUILD_XMREGCORE_TESTS)
    enable_testing()
    add_subdirectory(ext/googletest)
    add_subdirectory(tests)
endif()


