cmake_minimum_required(VERSION 3.10)

project(alt-package)

# Add include directory to the build
include_directories(${CMAKE_SOURCE_DIR}/include)

# Create a static library target
add_library(alt-package STATIC 
    src/hashing.c
    src/package.c
    src/xor.c
)

# Allow other targets to include our headers
target_include_directories(alt-package PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Add an option to conditionally build the example
option(BUILD_EXAMPLE "Build example" OFF)

if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()