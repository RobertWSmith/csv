cmake_minimum_required(VERSION 3.9)
project(csv VERSION ${CSV_VERSION} LANGUAGES C)

configure_file(
  ${CSV_PUBLIC_INCLUDE_DIR}/csv/version.h.in
  ${CSV_PUBLIC_INCLUDE_DIR}/csv/version.h
)

set(CSV_SOURCES
  csv_dialect.c
  csv_read.c
  csv_write.c
  CACHE FILEPATH "CSV Library source files" FORCE)

add_library(csv ${CSV_SOURCES})

target_link_libraries(csv PUBLIC zf_log)
target_compile_features(csv PUBLIC c_std_11)

set(CSV_PUBLIC_HEADER_FILES
  csv.h
  csv/definitions.h
  csv/dialect.h
  csv/read.h
  csv/stream.h
  csv/version.h
  csv/write.h
CACHE FILEPATH "CSV Library public header files" FORCE)

set(CSV_PRIVATE_HEADER_FILES
  dialect_private.h
  CACHE FILEPATH "CSV Library private header files" FORCE)

set_target_properties(csv PROPERTIES
    PUBLIC_HEADER  ${CSV_PUBLIC_HEADER_FILES}
    PRIVATE_HEADER ${CSV_PRIVATE_HEADER_FILES})

target_include_directories(csv
  PUBLIC
    $<BUILD_INTERFACE:${CSV_PROJECT_ROOT_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# CONFIG <= "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
# optimization level

# target_compile_options(csv
#   PUBLIC -Wall
#   PUBLIC $<$<BOOL:BUILD_VERBOSE>:-v>
#   PUBLIC $<$<CONFIG:Debug>:-g>
#   PUBLIC $<$<CONFIG:RelWithDebInfo>:-Og>
#   PUBLIC $<$<CONFIG:Release>:-O2>
#   PUBLIC $<$<CONFIG:MinSizeRel>:-Os>
# )
#
# target_compile_definitions(csv
#   PUBLIC $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:NDEBUG>
#   PUBLIC $<$<BOOL:MINGW>:__USE_MINGW_ANSI_STDIO=1>
# )

install(
  TARGETS csv
  EXPORT csv
  ARCHIVE DESTINATION lib/
  LIBRARY DESTINATION lib/
  RUNTIME DESTINATION bin/
  PUBLIC_HEADER DESTINATION include/
  PRIVATE_HEADER DESTINATION lib/
)
