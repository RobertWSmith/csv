cmake_minimum_required(VERSION 3.10)
project(csv-tests VERSION ${csv_VERSION} LANGUAGES C)

if(BUILD_TESTING)

  enable_testing()
  include(CTest)
  # might not need this
  include(CTestConfig)

  set(csv_TEST_DIR ${CMAKE_BINARY_DIR}/tests CACHE PATH "" FORCE)
  set(csv_TEST_TARGETS)

  create_test_sourcelist(
    all_csv_tests test_suite_csv.c
    test_dialect.c test_field.c test_read.c test_record.c test_write.c
  )

  add_executable(test_runner_csv ${all_csv_tests})
  target_link_libraries(test_runner_csv PUBLIC csv unity)
  # need interanl headers for some tests
  target_include_directories(test_runner_csv PUBLIC ${csv_PRIVATE_INCLUDE_DIR})

  set(tests_to_run ${all_csv_tests})
  remove(tests_to_run test_suite_csv.c)
  set(csv_TEST_TARGETS)

  foreach(test ${tests_to_run})
    get_filename_component(test_name ${test} NAME_WE)
    add_test(
      NAME ${test_name}
      COMMAND ${csv_TEST_DIR}/test_runner_csv ${test_name}
    )
    list(APPEND csv_TEST_TARGETS ${test_name})
  endforeach()

  add_custom_command(
    TARGET test_runner_csv
    POST_BUILD
      COMMAND ${CMAKE_CTEST_COMMAND} ARGS -C $<CONFIGURATION> --output-on-failure
      WORKING_DIRECTORY ${csv_TEST_DIR}
      COMMENT "Run All CSV Library Tests"
  )


  # add_executable(test_dialect test_dialect.c)
  # list(APPEND csv_TEST_TARGETS test_dialect)
  # target_include_directories(test_dialect PUBLIC ${csv_PRIVATE_INCLUDE_DIR})
  # target_link_libraries(test_dialect PUBLIC csv unity)
  # add_test(
  #   NAME test_dialect
  #   COMMAND ${csv_TEST_DIR}/test_dialect
  #   WORKING_DIRECTORY ${csv_TEST_DIR}
  # )
  #
  # add_executable(test_field test_field.c)
  # list(APPEND csv_TEST_TARGETS test_field)
  # target_link_libraries(test_field PUBLIC csv unity)
  # add_test(
  #   NAME test_field
  #   COMMAND ${csv_TEST_DIR}/test_field
  #   WORKING_DIRECTORY ${csv_TEST_DIR}
  # )
  #
  # add_executable(test_read test_read.c)
  # list(APPEND csv_TEST_TARGETS test_read)
  # target_link_libraries(test_read PUBLIC csv unity)
  # add_test(
  #   NAME test_read
  #   COMMAND ${csv_TEST_DIR}/test_read
  #   WORKING_DIRECTORY ${csv_TEST_DIR}
  # )
  #
  # add_executable(test_record test_record.c)
  # list(APPEND csv_TEST_TARGETS test_record)
  # target_link_libraries(test_record PUBLIC csv unity)
  # add_test(
  #   NAME test_record
  #   COMMAND ${csv_TEST_DIR}/test_record
  #   WORKING_DIRECTORY ${csv_TEST_DIR}
  # )
  #
  # add_executable(test_write test_write.c)
  # list(APPEND csv_TEST_TARGETS test_write)
  # target_link_libraries(test_write PUBLIC csv unity)
  # add_test(
  #   NAME test_write
  #   COMMAND ${csv_TEST_DIR}/test_write
  #   WORKING_DIRECTORY ${csv_TEST_DIR}
  # )
  #
  # add_custom_target(
  #   all_tests ALL
  #   COMMAND ${CMAKE_CTEST_COMMAND} -C ${CMAKE_CFG_INTDIR} -O ${csv_TEST_DIR}/test_csv.log
  #   BYPRODUCTS ${csv_TEST_DIR}/test_csv.log
  #   WORKING_DIRECTORY ${csv_TEST_DIR}
  #   COMMENT "Combine all tests into a single target"
  #   DEPENDS ${csv_TEST_TARGETS}
  # )
  #
  # add_custom_command(
  #   OUTPUT ${csv_TEST_DIR}/test_csv.log
  #   COMMAND ${CMAKE_CTEST_COMMAND} -C ${CMAKE_CFG_INTDIR} -O ${csv_TEST_DIR}/test_csv.log
  # )
  #
  # # add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/SubdirTests.log
  # #   ${CMAKE_CTEST_COMMAND} -C ${CMAKE_CFG_INTDIR} -O
  # # ${CMAKE_BINARY_DIR}/SubdirTests.log
  # #   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Subdir
  # # )
  #
  # # inspiration
  # # https://stackoverflow.com/questions/15115075/how-to-run-ctest-after-building-my-project-with-cmake
  # # add_custom_target(all_tests ALL DEPENDS ${csv_TEST_TARGETS})
  # # add_custom_command(
  # #   TARGET all_tests
  # #   POST_BUILD
  # #     COMMAND ${CMAKE_CTEST_COMMAND} ARGS -C $<CONFIGURATION> --output-on-failure
  # #     WORKING_DIRECTORY ${csv_TEST_DIR}
  # #     COMMENT "Run All CSV Library Tests"
  # # )

endif()
