project(FLogFS)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH})

enable_testing()

# Add a CMake option to include or exclude the examples directory
option(BUILD_EXAMPLES "Build the examples directory" OFF)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

add_subdirectory(test)
