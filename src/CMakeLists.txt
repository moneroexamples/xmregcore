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

# make static library called libmyxrm
# that we are going to link to
# in the root CMakeLists.txt file
add_library(myxrmcore
            STATIC
            ${SOURCE_FILES})

# include monero headers
target_include_monero_directories(myxrmcore)
