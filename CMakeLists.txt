project(FLogFS)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH})

enable_testing()

add_subdirectory(examples)
add_subdirectory(test)
