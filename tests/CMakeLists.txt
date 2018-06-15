cmake_minimum_required(VERSION 3.10)
project(csv VERSION ${csv_VERSION} LANGUAGES C)

enable_testing()
include(CTest)

set(csv_TEST_DIR ${CMAKE_BINARY_DIR}/tests CACHE PATH "" FORCE)

set(csv_TEST_FILES
  "test_dialect.c"
  "test_field.c"
  "test_read.c"
  "test_record.c"
  "test_write.c"
  CACHE FILEPATH "" FORCE)
set(csv_TEST_TARGETS)

foreach(test_file ${csv_TEST_FILES})
  get_filename_component(test_name ${test_file} NAME_WE)
  list(APPEND csv_TEST_TARGETS ${test_name})

  add_executable(${test_name} ${test_file})
  target_link_libraries(${test_name} PUBLIC csv unity)

  if(${test_file} STREQUAL "test_dialect.c")
    # need to include private headers because no public struct definition
    target_include_directories(${test_name} PUBLIC ${csv_PRIVATE_INCLUDE_DIR})
  endif()

  add_test(NAME ${test_name}
    COMMAND ${csv_TEST_DIR}/${test_name}
    WORKING_DIRECTORY ${csv_TEST_DIR})

  add_custom_command(
    OUTPUT _${test_name}_completed
    COMMAND ctest -C $<CONFIGURATION> --output-on-failure
    COMMAND cmake -E touch _${test_name}_completed
    DEPENDS ${test_name}
  )

  add_custom_target(
    _${test_name}_force ALL
    DEPENDS _${test_name}_completed
  )
endforeach()
