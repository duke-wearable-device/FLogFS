set(ARDUINO_IDE "${CMAKE_CURRENT_SOURCE_DIR}/../../../arduino-1.8.3")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../../arduino-cmake-minimal/cmake/")

include(ArduinoMinimal)

set(LIBRARIES SPI)

file(GLOB SOURCE_FILES *.cpp ../../src/*.cpp ../../backends/arduino-sd/*.cpp)

arduino(arduino-zero-flogfs "${SOURCE_FILES}" "${LIBRARIES}")

add_definitions("-DFLOG_ERASE_ZERO")

target_include_directories(arduino-zero-flogfs PUBLIC ./ ../../src ../../backends/arduino-sd)
