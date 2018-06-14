cmake_minimum_required(VERSION 3.11)
project(csv VERSION ${csv_VERSION} LANGUAGES C)

set(csv_PUBLIC_INCLUDE_DIR ${csv_PROJECT_ROOT_DIR}/include CACHE PATH "" FORCE)
set(csv_PRIVATE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "" FORCE)

configure_file(
  ${csv_PUBLIC_INCLUDE_DIR}/csv/version.h.in
  ${csv_PUBLIC_INCLUDE_DIR}/csv/version.h
)

add_library(csv csv_dialect.c)

target_compile_features(csv PUBLIC c_std_99)

set_target_properties(csv PROPERTIES
    PUBLIC_HEADER
      "csv.h;csv/definitions.h;csv/dialect.h;csv/field.h;csv/read.h;csv/record.h;csv/version.h;csv/write.h"
    PRIVATE_HEADER
      "dialect_private.h"
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
