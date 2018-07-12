cmake_minimum_required(VERSION 3.9)
project(csv_tests VERSION ${CSV_VERSION} LANGUAGES C)

set(CSV_WORKING_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "CSV library tests working directory" FORCE)
set(CSV_TEST_DIR ${CMAKE_BINARY_DIR}/tests CACHE PATH "CSV library tests binary directory" FORCE)
set(CSV_TEST_SOURCES
  test_dialect.c
  test_read.c
  test_write.c
CACHE FILEPATH "CSV Library source files for tests" FORCE)
# set(CSV_TEST_TARGETS)

foreach(source ${CSV_TEST_SOURCES})
  get_filename_component(targetname ${source} NAME_WE)
  add_executable(${targetname} ${source})
  target_link_libraries(${targetname} PUBLIC csv unity)

  if(${source} STREQUAL "test_dialect.c")
    target_include_directories(${targetname} PUBLIC ${CSV_PRIVATE_INCLUDE_DIR})
  endif()

  # target_compile_options(${targetname}
  #   PUBLIC -Wall
  #   PUBLIC $<$<BOOL:BUILD_VERBOSE>:-v>
  #   PUBLIC $<$<CONFIG:Debug>:-g
  #   PUBLIC $<$<CONFIG:RelWithDebInfo>:-Og>
  #   PUBLIC $<$<CONFIG:Release>:-O2>
  #   PUBLIC $<$<CONFIG:MinSizeRel>:-Os>
  # )

  # target_compile_definitions(${targetname}
  #   PUBLIC $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:NDEBUG>
  #   PUBLIC $<$<BOOL:MINGW>:__USE_MINGW_ANSI_STDIO=1>
  # )

  add_test(${targetname} ${CSV_TEST_DIR}/${targetname})
  set_tests_properties(${targetname} PROPERTIES
    WORKING_DIRECTORY ${CSV_WORKING_DIR}
    PASS_REGULAR_EXPRESSION "PASS"
    FAIL_REGULAR_EXPRESSION "FAIL"
    TIMEOUT 120
  )
endforeach()
