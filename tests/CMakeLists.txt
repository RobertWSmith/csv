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
  target_link_libraries(${targetname}
    PUBLIC csv
    PUBLIC zf_log
    PUBLIC unity)

  # need private header for testing to validate getters
  if(${targetname} MATCHES "[tT][eE][sS][tT]_[dD][iI][aA][lL][eE][cC][tT]")
    message(STATUS "${targetname} - adding private headers")
    target_include_directories(${targetname} PUBLIC ${CSV_PRIVATE_INCLUDE_DIR})
  else()
    message(STATUS "${targetname} - not adding private headers")
  endif()

  # # cmake/ClangTidy.cmake
  # if(CLANG_TIDY_FOUND)
  #   set_target_properties(${targetname} PROPERTIES C_CLANG_TIDY ${DO_CLANG_TIDY})
  # endif()
  target_compile_options(${targetname}
    PUBLIC $<$<C_COMPILER_ID:GNU>:-Wall;-Wextra;-Wshadow;-pedantic;-Wno-format>
    PUBLIC $<$<C_COMPILER_ID:Clang>:-Weverything;-pedantic;$<$<BOOL:BUILD_VERBOSE>:-v>>
    PUBLIC $<$<C_COMPILER_ID:MSVC>:/W4>)

  target_compile_definitions(${targetname}
    PUBLIC $<$<BOOL:MINGW>:__USE_MINGW_ANSI_STDIO=1>
    PUBLIC ZF_LOG_LEVEL=${CSV_LOG_LEVEL})

  add_test(${targetname} ${CSV_TEST_DIR}/${targetname})
  set_tests_properties(${targetname} PROPERTIES
    WORKING_DIRECTORY ${CSV_WORKING_DIR}
    PASS_REGULAR_EXPRESSION "PASS"
    FAIL_REGULAR_EXPRESSION "FAIL"
    TIMEOUT 120
  )
endforeach()
