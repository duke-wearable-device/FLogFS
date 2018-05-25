set(CMAKE_BUILD_TYPE Debug)

set(FLOGS_SRCS ../../src/flogfs.cpp)

add_executable(example-linux-mmap-ff main.cpp ../../backends/linux-mmap/flogfs_linux_mmap.cpp ${FLOGS_SRCS})
target_include_directories(example-linux-mmap-ff PUBLIC ./ ../../inc ../../backends/linux-mmap)
set_target_properties(example-linux-mmap-ff PROPERTIES CXX_STANDARD 11)


add_executable(example-linux-mmap-00 main.cpp ../../backends/linux-mmap/flogfs_linux_mmap.cpp ${FLOGS_SRCS})
target_include_directories(example-linux-mmap-00 PUBLIC ./ ../../inc ../../backends/linux-mmap)
target_compile_definitions(example-linux-mmap-00 PUBLIC "-DFLOG_ERASE_ZERO")
set_target_properties(example-linux-mmap-00 PROPERTIES CXX_STANDARD 11)
