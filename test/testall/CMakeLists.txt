set(FLOGFS_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
set(FLOGFS_BACKEND_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../backends/linux-mmap)

file(GLOB SRCS *.cpp ${FLOGFS_SRCS}/*.cpp ${FLOGFS_BACKEND_SRCS}/*.cpp)

add_executable(testall ${SRCS})

set(PROJECT_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${FLOGFS_SRCS} ${FLOGFS_BACKEND_SRCS}
)

target_include_directories(testall PUBLIC "${PROJECT_INCLUDES}")

target_link_libraries(testall libgtest libgmock)

set_target_properties(testall PROPERTIES C_STANDARD 11)
set_target_properties(testall PROPERTIES CXX_STANDARD 11)

add_test(NAME testall COMMAND testall)
