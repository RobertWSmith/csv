cmake_minimum_required(VERSION 3.10)
project(csv_external VERSION ${csv_VERSION} LANGUAGES C)

find_package(Git REQUIRED)

if(NOT GIT_FOUND)
  message("Could not find git, cannot build tests")
else()
  set(Unity_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Unity CACHE PATH "" FORCE)

  execute_process(
    COMMAND ${GIT_EXECUTABLE} submodule update --init -- external/Unity
    WORKING_DIRECTORY ${csv_PROJECT_ROOT_DIR}
  )
  # tagged on 2017-11-14, specified to ensure no update surprises
  # use this checksum hash if the tags are ever modified
  # "287e076962ec711cd2bdf08364a8df9ce51e106b"
  execute_process(
    COMMAND ${GIT_EXECUTABLE} checkout "v2.4.3"
    WORKING_DIRECTORY ${Unity_ROOT}
  )

  add_library(unity ${Unity_ROOT}/src/unity.c)
  target_include_directories(unity PUBLIC ${Unity_ROOT}/src)
  target_compile_features(unity PUBLIC c_std_99)
  target_compile_options(unity PRIVATE -Wall -Og)
endif()
