cmake_minimum_required(VERSION 3.9)
project(csv_external VERSION ${CSV_VERSION} LANGUAGES C)

find_package(Git REQUIRED)

if(NOT GIT_FOUND)
  message("Could not find git, cannot build tests")
else()
  set(UNITY_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Unity
    CACHE PATH "Source path for Unity unit testing library" FORCE)

  execute_process(
    COMMAND ${GIT_EXECUTABLE} submodule update --init -- external/Unity
    WORKING_DIRECTORY ${CSV_PROJECT_ROOT_DIR}
  )
  # tagged on 2017-11-14, specified to ensure no update surprises
  # use this checksum hash if the tags are ever modified
  # "287e076962ec711cd2bdf08364a8df9ce51e106b"
  execute_process(
    COMMAND ${GIT_EXECUTABLE} checkout "v2.4.3"
    WORKING_DIRECTORY ${UNITY_ROOT}
  )

  # force static linkage for Unity, it's small and MinGW seems to have issues
  # dynamically linking for some undetermined reason
  add_library(unity STATIC ${UNITY_ROOT}/src/unity.c)
  target_include_directories(unity PUBLIC ${UNITY_ROOT}/src)
  target_compile_features(unity PUBLIC c_std_11)
  target_compile_options(unity PRIVATE -Wall;-pedantic)

  set(ZF_LOG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/zf_log
    CACHE PATH "Source path for ZF Log logging library" FORCE)

  execute_process(
    COMMAND ${GIT_EXECUTABLE} submodule update --init -- external/zf_log
    WORKING_DIRECTORY ${CSV_PROJECT_ROOT_DIR}
  )
  # tagged on 2017-11-14, specified to ensure no update surprises
  # use this checksum hash if the tags are ever modified
  # "287e076962ec711cd2bdf08364a8df9ce51e106b"
  execute_process(
    COMMAND ${GIT_EXECUTABLE} checkout "fb22f401c07fcea711c55f1300ae981938053141"
    WORKING_DIRECTORY ${ZF_LOG_ROOT}
  )

  set(ZF_LOG_INCLUDE_DIR ${ZF_LOG_ROOT}/zf_log
    CACHE PATH "ZF Log library public header directory" FORCE)

  add_library(zf_log STATIC ${ZF_LOG_ROOT}/zf_log/zf_log.c)
  target_include_directories(zf_log PUBLIC ${ZF_LOG_ROOT}/zf_log)
  target_compile_features(zf_log PUBLIC c_std_11)
endif()
