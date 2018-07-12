cmake_minimum_required(VERSION 3.9)
project(csv VERSION ${csv_VERSION} LANGUAGES C)

configure_file(
  ${csv_PUBLIC_INCLUDE_DIR}/csv/version.h.in
  ${csv_PUBLIC_INCLUDE_DIR}/csv/version.h
)

add_library(csv csv_dialect.c csv_read.c csv_write.c)

target_link_libraries(csv PUBLIC zf_log)

target_compile_features(csv PUBLIC c_std_11)

set(csv_PUBLIC_HEADER_FILES
  csv.h
  csv/definitions.h
  csv/dialect.h
  csv/read.h
  csv/stream.h
  csv/version.h
  csv/write.h
  CACHE FILEPATH "" FORCE)

set(csv_PRIVATE_HEADER_FILES
  dialect_private.h
  CACHE FILEPATH "" FORCE)

set_target_properties(csv PROPERTIES
    PUBLIC_HEADER
      "${csv_PUBLIC_HEADER_FILES}"
    PRIVATE_HEADER
      "${csv_PRIVATE_HEADER_FILES}"
)

target_include_directories(csv
  PUBLIC
    $<BUILD_INTERFACE:${csv_PUBLIC_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    $<BUILD_INTERFACE:${csv_PRIVATE_INCLUDE_DIR}>
)

target_compile_options(csv
  PUBLIC
    $<IF:$<CONFIG:Release>,-O2,-Og>;-Wall;-Wextra;-Wpointer-arith;-Wcast-align;
      -Wwrite-strings;-Wunreachable-code;-Winit-self;
      -Wmissing-field-initializers;-Wno-unknown-pragmas;-Wstrict-prototypes;
      -Wundef;-Wold-style-definition
)

if (MINGW)
  target_compile_definitions(csv PUBLIC __USE_MINGW_ANSI_STDIO=1)
endif()

install(
  TARGETS csv
  EXPORT csv
  ARCHIVE DESTINATION lib/
  LIBRARY DESTINATION lib/
  RUNTIME DESTINATION bin/
  PUBLIC_HEADER DESTINATION include/
  PRIVATE_HEADER DESTINATION lib/
)
