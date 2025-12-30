cmake_minimum_required(VERSION 3.10)
project(tlsf-allocator VERSION 1.0.0 LANGUAGES C)

# Options
option(TLSF_BUILD_TESTS "Build TLSF tests" ON)
option(TLSF_BUILD_EXAMPLES "Build TLSF examples" ON)
option(TLSF_BUILD_SHARED "Build TLSF as shared library" OFF)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Main library
if(TLSF_BUILD_SHARED)
    add_library(tlsf SHARED 
        src/tlsf.c
        src/tlsf_block.c
        src/tlsf_mapping.c
        src/tlsf_bitmap.c
        platform/tlsf_platform.c
    )
else()
    add_library(tlsf STATIC
        src/tlsf.c
        src/tlsf_block.c
        src/tlsf_mapping.c
        src/tlsf_bitmap.c
        platform/tlsf_platform.c
    )
endif()

# Examples
if(TLSF_BUILD_EXAMPLES)
    add_executable(example_simple examples/simple_usage.c)
    target_link_libraries(example_simple tlsf)
endif()

# Tests
if(TLSF_BUILD_TESTS)
    enable_testing()
    
    # Basic tests
    add_executable(test_basic tests/test_basic.c)
    target_link_libraries(test_basic tlsf)
    add_test(NAME test_basic COMMAND test_basic)
    
    # Stress tests
    add_executable(test_stress tests/test_stress.c)
    target_link_libraries(test_stress tlsf)
    add_test(NAME test_stress COMMAND test_stress)
    
    # Fragmentation tests
    add_executable(test_fragmentation tests/test_fragmentation.c)
    target_link_libraries(test_fragmentation tlsf)
    add_test(NAME test_fragmentation COMMAND test_fragmentation)
endif()

# Installation
install(TARGETS tlsf
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)
