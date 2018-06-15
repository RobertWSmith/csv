cmake_minimum_required(VERSION 3.10)
project(csv VERSION ${csv_VERSION} LANGUAGES C)

configure_file(
  ${csv_PUBLIC_INCLUDE_DIR}/csv/version.h.in
  ${csv_PUBLIC_INCLUDE_DIR}/csv/version.h
)

add_library(csv csv_dialect.c csv_field.c csv_read.c csv_record.c csv_write.c)

target_compile_features(csv PUBLIC c_std_99)

set(csv_PUBLIC_HEADER_FILES
  csv.h
  csv/definitions.h
  csv/dialect.h
  csv/field.h
  csv/read.h
  csv/record.h
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
      -Wwrite-strings;-Wswitch-default;-Wunreachable-code;-Winit-self;
      -Wmissing-field-initializers;-Wno-unknown-pragmas;-Wstrict-prototypes;
      -Wundef;-Wold-style-definition
)

install(
  TARGETS csv
  EXPORT csv
  ARCHIVE DESTINATION lib/
  LIBRARY DESTINATION lib/
  RUNTIME DESTINATION bin/
  PUBLIC_HEADER DESTINATION include/
  PRIVATE_HEADER DESTINATION lib/
)
