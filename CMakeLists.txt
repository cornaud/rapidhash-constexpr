#
#	Copyright(c) 2026 Nathan Cornaud
#
#	SPDX-License-Identifier: MIT
#

cmake_minimum_required(VERSION 3.21)
project(rapidhash_cexpr CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# Test 1 (RAPIDHASH_UNROLLED + RAPIDHASH_CEXPR_ALLOW_CLASS_REPR only).
add_executable(test_base tests/dummy_main.cpp)
target_compile_definitions(test_base PRIVATE
    RAPIDHASH_UNROLLED
    RAPIDHASH_CEXPR_ALLOW_CLASS_REPR
)
target_include_directories(test_base PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_test(NAME test_base COMMAND test_base)

# Test 2 (RAPIDHASH_UNROLLED + RAPIDHASH_CEXPR_ALLOW_CLASS_REPR + RAPIDHASH_PROTECTED).
add_executable(test_protected tests/dummy_main.cpp)
target_compile_definitions(test_protected PRIVATE
    RAPIDHASH_UNROLLED
    RAPIDHASH_CEXPR_ALLOW_CLASS_REPR
    RAPIDHASH_PROTECTED
)
target_include_directories(test_protected PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_test(NAME test_protected COMMAND test_protected)

# Test 3 (RAPIDHASH_UNROLLED + RAPIDHASH_CEXPR_ALLOW_CLASS_REPR + RAPIDHASH_CEXPR_FORCE_PORTABLE_MUL).
add_executable(test_portable tests/dummy_main.cpp)
target_compile_definitions(test_portable PRIVATE
    RAPIDHASH_UNROLLED
    RAPIDHASH_CEXPR_ALLOW_CLASS_REPR
    RAPIDHASH_CEXPR_FORCE_PORTABLE_MUL
)
target_include_directories(test_portable PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_test(NAME test_portable COMMAND test_portable)