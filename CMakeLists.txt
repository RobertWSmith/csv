cmake_minimum_required(VERSION 3.11)

set(csv_VERSION_MAJOR 0)
set(csv_VERSION_MINOR 0)
set(csv_VERSION_PATCH 0)
set(csv_VERSION_TWEAK 0)
set(csv_VERSION "${csv_VERSION_MAJOR}.${csv_VERSION_MINOR}.${csv_VERSION_PATCH}")

project(csv_root VERSION ${csv_VERSION} LANGUAGES C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "" FORCE)

# check for default install prefix, or assign build directory if undefined
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTAL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "" FORCE)
  message("No default install directory defied, using default of: `${CMAKE_INSTALL_PREFIX}`")
endif()

set(csv_PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "" FORCE)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(tests)
