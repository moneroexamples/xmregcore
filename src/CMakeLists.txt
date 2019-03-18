cmake_minimum_required(VERSION 3.2)

project(myxrmcore)

set(SOURCE_FILES
        MicroCore.h
        MicroCore.cpp
        tools.h
        tools.cpp
        UniversalIdentifier.hpp
        UniversalIdentifier.cpp
        Account.h
        Account.cpp)

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

find_package(HIDAPI)

set(LIBRARIES
        Monero::Monero
        ${Boost_LIBRARIES}
        ${HIDAPI_LIBRARIES}
        sodium
        pthread
        unbound
        curl        
        ssl
        crypto
        dl
        atomic
        unwind)

# make static library called libmyxrm
# that we are going to link to
# in the root CMakeLists.txt file
add_library(myxrmcore
            STATIC
            ${SOURCE_FILES})

target_link_libraries(myxrmcore
    PUBLIC ${LIBRARIES})

add_library(XMREG::core ALIAS myxrmcore)

