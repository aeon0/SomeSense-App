cmake_minimum_required (VERSION 3.5)
project(Ilona)

set(${PROJECT_NAME}_VERSION_MAJOR 0)
set(${PROJECT_NAME}_VERSION_MINOR 1)

include(compiler_flags.cmake)

# some options for the build
option(INSTALL_DEPENDENCIES "Only install dependencies" OFF)
option(BUILD_TEST "Build the unit tests" OFF)
option(BUILD_SIM "Build simulation (Carla client)" OFF)
# export BUILD_SIM to c++ code as compiler definition
if (BUILD_SIM)
  add_compile_definitions(BUILD_SIM)
endif()

# we want to choice our own prefixes
set(CMAKE_STATIC_LIBRARY_PREFIX "")

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(COMPONENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components")
set(INSTALL_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/bin/${CMAKE_BUILD_TYPE}")
set(INSTALL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/lib/${CMAKE_BUILD_TYPE}")
set(INSTALL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/include")
set(EXTERNAL_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals/install")

include_directories(${COMPONENT_DIR} NO_DEFAULT_PATH)

include(externals/dependencies.cmake)

if (NOT INSTALL_DEPENDENCIES)
  add_subdirectory(components/frame)
  add_subdirectory(components/utilities)
  add_subdirectory(components/com_out)
  add_subdirectory(components/data_reader)
  add_subdirectory(components/serialize)
  # algos
  add_subdirectory(components/algo/optical_flow)
  add_subdirectory(components/algo/example)

  if(BUILD_TEST)
    add_subdirectory(test)
  endif()
endif()
