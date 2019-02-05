cmake_minimum_required(VERSION 3.2)

set(PROJECT_NAME
        xmregcore)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED ON)

if (NOT MONERO_DIR)
    set(MONERO_DIR ~/monero)
endif()

option(BUILD_XMREGCORE_TESTS "Build tests for the project" ON)

message(STATUS MONERO_DIR ": ${MONERO_DIR}")

set(MONERO_SOURCE_DIR ${MONERO_DIR}
        CACHE PATH "Path to the root directory for Monero")

# set location of monero build tree
set(MONERO_BUILD_DIR ${MONERO_SOURCE_DIR}/build/release/
        CACHE PATH "Path to the build directory for Monero")

set(MY_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake"
        CACHE PATH "The path to the cmake directory of the current project")

list(APPEND CMAKE_MODULE_PATH "${MY_CMAKE_DIR}")

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${MONERO_BUILD_DIR}"
        CACHE PATH "Add Monero directory for library searching")

include(MyUtils)

include(CodeCoverage)
APPEND_COVERAGE_COMPILER_FLAGS()
set(COVERAGE_EXCLUDES
       '${CMAKE_SOURCE_DIR}/ext/*'
       '${MONERO_SOURCE_DIR}/*'
       '${CMAKE_SOURCE_DIR}/tests/*')


find_package(Monero)

# find boost
find_package(Boost COMPONENTS
        system
        filesystem
        thread
        date_time
        chrono
        regex
        serialization
        program_options
        REQUIRED)


# add src/ subfolder
add_subdirectory(src/)


set(SOURCE_FILES
        main.cpp)

add_executable(${PROJECT_NAME}
        ${SOURCE_FILES})


# include boost headers
target_include_directories(${PROJECT_NAME}
    PRIVATE ${Boost_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME}
    PRIVATE ${MONERO_SOURCE_DIR}/build)

# include monero headers
target_include_monero_directories(${PROJECT_NAME})

set(LIBRARIES
        myxrmcore
        ${Monero_LIBRARIES}
        ${Boost_LIBRARIES}
        sodium
        pthread
        unbound
        curl        
        ssl
        crypto)

if(APPLE)
    set(LIBRARIES ${LIBRARIES} "-framework IOKit -framework Foundation")
else()
    set(LIBRARIES ${LIBRARIES} atomic)
endif()

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT WIN32)
    set(LIBRARIES ${LIBRARIES} unwind)
endif()


if (WIN32)
    set(LIBRARIES ${LIBRARIES}
            wsock32
            ntdll
            ws2_32
            Iphlpapi)
else()
    set(LIBRARIES ${LIBRARIES} dl)
endif()

find_package(HIDAPI)
set(LIBRARIES ${LIBRARIES} ${HIDAPI_LIBRARIES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE ${LIBRARIES})

if (BUILD_XMREGCORE_TESTS)
    enable_testing()
    add_subdirectory(ext/googletest)
    add_subdirectory(tests)
endif()

