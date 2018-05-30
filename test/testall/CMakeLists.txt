set(FLOGFS_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
set(FLOGFS_BACKEND_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../backends/linux-mmap)

file(GLOB SRCS *.cpp ${FLOGFS_SRCS}/*.cpp ${FLOGFS_BACKEND_SRCS}/*.cpp)

set(PROJECT_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${FLOGFS_SRCS} ${FLOGFS_BACKEND_SRCS}
)

add_executable(testall-ff ${SRCS})
target_include_directories(testall-ff PUBLIC "${PROJECT_INCLUDES}")
target_link_libraries(testall-ff libgtest libgmock)
set_target_properties(testall-ff PROPERTIES C_STANDARD 11)
set_target_properties(testall-ff PROPERTIES CXX_STANDARD 11)
add_test(NAME testall-ff COMMAND testall-ff)

add_executable(testall-00 ${SRCS})
target_compile_definitions(testall-00 PUBLIC "-DFLOGFS_ERASE_ZERO")
target_include_directories(testall-00 PUBLIC "${PROJECT_INCLUDES}")
target_link_libraries(testall-00 libgtest libgmock)
set_target_properties(testall-00 PROPERTIES C_STANDARD 11)
set_target_properties(testall-00 PROPERTIES CXX_STANDARD 11)
add_test(NAME testall-00 COMMAND testall-00)
