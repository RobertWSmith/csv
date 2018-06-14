cmake_minimum_required(VERSION 3.11)
project(csv VERSION ${csv_VERSION} LANGUAGES C)

enable_testing()
include(CTest)

add_executable(test_dialect test_dialect.c)
target_link_libraries(test_dialect PUBLIC csv unity)
# need to include private headers because no public struct definition
target_include_directories(test_dialect ${csv_PRIVATE_INCLUDE_DIR})
add_test(NAME test_dialect COMMAND test_dialect)

add_custom_target(all_tests ALL DEPENDS test_dialect)

add_custom_command(
  TARGET all_tests
  COMMENT "Run all tests"
  POST_BUILD COMMAND ctest ARGS --output-on-failure
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
