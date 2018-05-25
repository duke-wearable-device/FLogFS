set(CMAKE_BUILD_TYPE Debug)

set(FLOGS_SRCS ../../src/flogfs.cpp)

add_executable(testing main.cpp ../../backends/linux-mmap/flogfs_linux_mmap.cpp ${FLOGS_SRCS})

target_include_directories(testing PUBLIC ./ ../../inc ../../backends/linux-mmap)

set_target_properties(testing PROPERTIES CXX_STANDARD 11)
