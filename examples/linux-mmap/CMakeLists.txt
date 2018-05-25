set(CMAKE_BUILD_TYPE Debug)

set(FLOGS_SRCS ../../src/flogfs.cpp)

add_executable(example-linux-mmap main.cpp ../../backends/linux-mmap/flogfs_linux_mmap.cpp ${FLOGS_SRCS})

target_include_directories(example-linux-mmap PUBLIC ./ ../../inc ../../backends/linux-mmap)

set_target_properties(example-linux-mmap PROPERTIES CXX_STANDARD 11)
