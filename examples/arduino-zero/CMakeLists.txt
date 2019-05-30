include(Arduino)

enable_arduino_toolchain()

file(GLOB sources *.cpp ../../src/*.cpp ../../backends/arduino-sd/*.cpp)

add_arduino_library(arduino-zero-flogfs "${sources}")

add_definitions("-DFLOGFS_ERASE_ZERO")
add_definitions("-DFLOGFS_DEBUG")

find_package(SPI)
target_link_libraries(arduino-zero-flogfs SPI)

target_include_directories(arduino-zero-flogfs PUBLIC ./ ../../src ../../backends/arduino-sd)

add_arduino_firmware(arduino-zero-flogfs)
