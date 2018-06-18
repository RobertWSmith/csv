cmake_minimum_required(VERSION 3.10)
project(csv-tests VERSION ${csv_VERSION} LANGUAGES C)

if(BUILD_TESTING)

  enable_testing()
  include(CTest)
  # might not need this
  include(CTestConfig)

  set(csv_WORKING_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "" FORCE)
  set(csv_TEST_DIR ${CMAKE_BINARY_DIR}/tests CACHE PATH "" FORCE)
  set(csv_TEST_SOURCES
    test_dialect.c
    test_read.c
    test_write.c
  CACHE FILEPATH "" FORCE)
  # set(csv_TEST_TARGETS)

  foreach(source ${csv_TEST_SOURCES})
    get_filename_component(targetname ${source} NAME_WE)
    add_executable(${targetname} ${source})
    target_link_libraries(${targetname} PUBLIC csv unity)
    if(${source} STREQUAL "test_dialect.c")
      target_include_directories(${targetname} PUBLIC ${csv_PRIVATE_INCLUDE_DIR})
    endif()

    add_test(${targetname} ${csv_TEST_DIR}/${targetname})
    set_tests_properties(${targetname} PROPERTIES
      WORKING_DIRECTORY ${csv_WORKING_DIR}
      PASS_REGULAR_EXPRESSION "PASS"
      FAIL_REGULAR_EXPRESSION "FAIL"
      TIMEOUT 120
    )
    # list(APPEND csv_TEST_TARGETS ${targetname})
  endforeach()

endif()
