
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable/Disable output of compile commands during generation." FORCE)

set(default_build_shared_libs ON CACHE BOOL "" FORCE)
if(WIN32)
  # for some reason, after building on windows executables can't find the linked library.
  set(default_build_shared_libs OFF CACHE BOOL "" FORCE)
endif()

set_property(CACHE default_build_shared_libs PROPERTY HELPSTRING
  "Default setting for CSV project for shared/static library build type")

# check for default install prefix, or assign build directory if undefined
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Install directory used by install()." FORCE)
  message("No default install directory defined, setting to default: `${CMAKE_INSTALL_PREFIX}`")
endif()
