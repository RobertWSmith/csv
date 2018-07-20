cmake_minimum_required(VERSION 3.9)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "CSV Project CMake Modules" FORCE)

include(GNUInstallDirs)

include(BuildType)
include(ClangTidy)

include(CSVVersion)
include(CSVConfig)

project(csv_root VERSION ${CSV_VERSION} LANGUAGES C)

##### Begin Build Variables
option(BUILD_VERBOSE     "Flag which determines if the compiler should run in verbose mode"           OFF)
option(BUILD_TESTING     "Flag which controls CTest execution for the CSV Unit Tests"                 ON)
option(BUILD_SHARED_LIBS "Should a shared (ON) or static (OFF) library be built?"                     ${default_build_shared_libs})

set(CSV_LOG_LEVEL "ZF_LOG_DEBUG" CACHE STRING "CSV Library logging level" FORCE)
set(CSV_TEST_LOG_LEVEL ${CSV_LOG_LEVEL} CACHE STRING "CSV Library testing log level" FORCE)

##### End Build Variables
set(CSV_PROJECT_ROOT_DIR    ${CMAKE_CURRENT_SOURCE_DIR}     CACHE PATH   "CSV Project root directory"           FORCE)
set(CSV_PUBLIC_INCLUDE_DIR  ${CSV_PROJECT_ROOT_DIR}/include CACHE PATH   "CSV Project public header directory"  FORCE)
set(CSV_PRIVATE_INCLUDE_DIR ${CSV_PROJECT_ROOT_DIR}/src     CACHE PATH   "CSV Project private header directory" FORCE)

include(CSVLogging)

message(STATUS "CSV Build Log Level:      ${CSV_LOG_LEVEL}")
message(STATUS "CSV Test Log Level:       ${CSV_TEST_LOG_LEVEL}")
message(STATUS "CSV Build Log Definition: ${CSV_LOGGING_DEFINITION}")
message(STATUS "CSV Test Log Definition:  ${CSV_TEST_LOGGING_DEFINITION}")

add_subdirectory(external)
add_subdirectory(src)

if(BUILD_TESTING)
  set(CTEST_PROJECT_NAME "CSV_Library")
  set(CTEST_BUILD_NAME "CSV_Library-${CSV_VERSION}")

  include(CTest)

  enable_testing()
  add_subdirectory(tests)
endif(BUILD_TESTING)

# copy the compile commands for clang to the source root
# configure_file(
#   ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
#   ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
#   COPYONLY
# )
