find_program(
  CLANG_TIDY
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable"
)

if(NOT CLANG_TIDY)
  set(CLANG_TIDY_FOUND OFF CACHE BOOL "" FORCE)
  message(STATUS "clang-tidy not found")
else()
  set(CLANG_TIDY_FOUND ON CACHE BOOL "" FORCE)
  message(STATUS "clang-tidy found: ${CLANG_TIDY}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*"
  CACHE STRING "Run clang-tidy with build" FORCE)
endif()

set_property(CACHE CLANG_TIDY_FOUND PROPERTY HELPSTRING
  "Indicates if clang-tidy was found in the current system")
