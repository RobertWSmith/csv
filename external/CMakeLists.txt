cmake_minimum_required(VERSION 3.11)
project(csv_external VERSION ${csv_VERSION} LANGUAGES C)

include(ExternalProject)

set(CSV_EXTERNAL_BUILD_DIR ${CMAKE_BINARY_DIR}/external CACHE PATH "" FORCE)
set(Unity_PREFIX_PATH ${CSV_EXTERNAL_BUILD_DIR}/Unity CACHE PATH "" FORCE)
set(Unity_ROOT ${Unity_PREFIX_PATH}/src/Unity/src CACHE PATH "" FORCE)

ExternalProject_Add(
  Unity
  PREFIX            ${Unity_PREFIX_PATH}
  GIT_REPOSITORY    "https://github.com/ThrowTheSwitch/Unity.git"
  GIT_TAG           "v2.4.3"
  UPDATE_COMMAND    ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

add_library(unity ${Unity_ROOT}/unity.c)
target_include_directories(unity PUBLIC ${Unity_ROOT})
target_compile_features(unity PUBLIC c_std_99)
target_compile_options(unity PRIVATE -Wall -Og)
