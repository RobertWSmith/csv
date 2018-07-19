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
option(BUILD_LOG_LEVEL   "One of None, Fatal, Error, Warn, Info, Debug or Verbose. Defaults to Debug" "Debug")

##### End Build Variables
set(CSV_PROJECT_ROOT_DIR    ${CMAKE_CURRENT_SOURCE_DIR}     CACHE PATH "CSV Project root directory"           FORCE)
set(CSV_PUBLIC_INCLUDE_DIR  ${CSV_PROJECT_ROOT_DIR}/include CACHE PATH "CSV Project public header directory"  FORCE)
set(CSV_PRIVATE_INCLUDE_DIR ${CSV_PROJECT_ROOT_DIR}/src     CACHE PATH "CSV Project private header directory" FORCE)

if (${BUILD_LOG_LEVEL} MATCHES "[nN][oO][nN][eE]")
  set(CSV_LOG_LEVEL ZF_LOG_NONE)
elseif (${BUILD_LOG_LEVEL} MATCHES "[fF][aA][tT][aA][lL]")
  set(CSV_LOG_LEVEL ZF_LOG_FATAL)
elseif (${BUILD_LOG_LEVEL} MATCHES "[eE][rR][rR][oO][rR]")
  set(CSV_LOG_LEVEL ZF_LOG_ERROR)
elseif (${BUILD_LOG_LEVEL} MATCHES "[wW][aA][rR][nN]")
  set(CSV_LOG_LEVEL ZF_LOG_WARN)
elseif (${BUILD_LOG_LEVEL} MATCHES "[iI][nN][fF][oO]")
  set(CSV_LOG_LEVEL ZF_LOG_INFO)
elseif (${BUILD_LOG_LEVEL} MATCHES "[dD][eE][bB][uU][gG]")
  set(CSV_LOG_LEVEL ZF_LOG_DEBUG)
elseif (${BUILD_LOG_LEVEL} MATCHES "[vV][eE][rR][bB][oO][sS][eE]")
  set(CSV_LOG_LEVEL ZF_LOG_VERBOSE)
else()
  set(CSV_LOG_LEVEL ZF_LOG_DEBUG)
endif()

message(STATUS "CSV Build Log Level: ${CSV_LOG_LEVEL}")

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
