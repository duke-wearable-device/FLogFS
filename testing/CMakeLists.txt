set(CMAKE_BUILD_TYPE Debug)

set(FLOGS_SRCS ../src/flogfs.cpp)

add_executable(testing main.cpp flogfs_linux.cpp ${FLOGS_SRCS})

target_include_directories(testing PUBLIC ./ ../inc)

set_target_properties(testing PROPERTIES CXX_STANDARD 11)
