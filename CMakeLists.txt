cmake_minimum_required(VERSION 3.10)

set(csv_VERSION_MAJOR 0)
set(csv_VERSION_MINOR 0)
set(csv_VERSION_PATCH 0)
set(csv_VERSION_TWEAK 0)
set(csv_VERSION "${csv_VERSION_MAJOR}.${csv_VERSION_MINOR}.${csv_VERSION_PATCH}")

project(csv_root VERSION ${csv_VERSION} LANGUAGES C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "" FORCE)

##### Begin Build Variables
option( BUILD_TESTING
        "Flag which controls CTest execution for the CSV Unit Tests"
        ON)

if(WIN32)
  # for some reason, after building on windows executables can't find the
  # linked library.
  set(BUILD_SHARED_LIBS OFF)
else()
  set(BUILD_SHARED_LIBS ON)
endif()

option( BUILD_SHARED_LIBS
        "Flag which controls if CSV Library is built as shared (ON) or static (OFF)"
        ${BUILD_SHARED_LIBS})
##### End Build Variables

include(GNUInstallDirs)

# check for default install prefix, or assign build directory if undefined
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "" FORCE)
  message("No default install directory defied, using default of: `${CMAKE_INSTALL_PREFIX}`")
endif()

set(csv_PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "" FORCE)
set(csv_PUBLIC_INCLUDE_DIR ${csv_PROJECT_ROOT_DIR}/include CACHE PATH "" FORCE)
set(csv_PRIVATE_INCLUDE_DIR ${csv_PROJECT_ROOT_DIR}/src CACHE PATH "" FORCE)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(tests)
