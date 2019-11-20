cmake_minimum_required (VERSION 3.5)
project(Ilona)

# some options for the build
option(BUILD_TEST "Build the unit tests" OFF)

# we want to choice our own prefix to prevent naming issues
set(CMAKE_STATIC_LIBRARY_PREFIX "")

set(COMPONENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components")
set(INSTALL_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/bin/${CMAKE_BUILD_TYPE}")
set(INSTALL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/lib/${CMAKE_BUILD_TYPE}")
set(INSTALL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/include")

include_directories(${COMPONENT_DIR})

add_subdirectory(externals)
add_subdirectory(components/data_reader)
add_subdirectory(components/example)
add_subdirectory(components/frame)

if(BUILD_TEST)
  add_subdirectory(test/frame)
endif()
