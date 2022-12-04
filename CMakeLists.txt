cmake_minimum_required(VERSION 3.23)
project(jnp1_organism)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wfatal-errors -Wall -Wextra -Wpedantic -O2"
        )

include_directories(.)

add_executable(jnp1_organism
#        organism_example.cc
        test.cpp
        )

add_custom_target(format
        COMMAND /usr/bin/clang-format
        -i
        organism.h
        )





