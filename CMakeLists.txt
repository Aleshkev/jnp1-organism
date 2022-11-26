cmake_minimum_required(VERSION 3.16)
project(jnp1_organism)

set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors -Wall -Wextra -O2")

add_executable(jnp1_organism
        organism_example.cc)

add_custom_target(
        format
        COMMAND /usr/bin/clang-format
        -i
        organism.h
)