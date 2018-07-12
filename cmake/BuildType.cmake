# set a default build type if not specified

set(default_build_type "Release" CACHE STRING "Default value for CMAKE_BUILD_TYPE if not defined" FORCE)
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug" CACHE STRING "Default value for CMAKE_BUILD_TYPE if not defined" FORCE)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Specifies the build type on single-configuration generators." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
