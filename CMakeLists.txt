cmake_minimum_required (VERSION 3.5)
project(Ilona)

set (${PROJECT_NAME}_VERSION_MAJOR 0)
set (${PROJECT_NAME}_VERSION_MINOR 1)

# some options for the build
option(BUILD_TEST "Build the unit tests" OFF)

# we want to choice our own prefix to prevent naming issues
set(CMAKE_STATIC_LIBRARY_PREFIX "")

set(COMPONENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components")
set(INSTALL_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/bin/${CMAKE_BUILD_TYPE}")
set(INSTALL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/lib/${CMAKE_BUILD_TYPE}")
set(INSTALL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/include")
set(EXTERNAL_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals/install")

include_directories(${COMPONENT_DIR} NO_DEFAULT_PATH)

include(externals/dependencies.cmake)

add_subdirectory(components/utilities)
add_subdirectory(components/data_reader)
add_subdirectory(components/frame)

if(BUILD_TEST)
  add_subdirectory(test/frame)
endif()
