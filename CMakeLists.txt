cmake_minimum_required (VERSION 3.10)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# Note: If project name is changed, also the task.json and launch.json for vscode needs to be updated
project(app)

set(${PROJECT_NAME}_VERSION_MAJOR 0)
set(${PROJECT_NAME}_VERSION_MINOR 1)

# some options for the build
option(INSTALL_DEPENDENCIES "Only install dependencies" OFF)
option(BUILD_TEST "Build the unit tests" OFF)
option(BUILD_SIM "Build simulation (Carla client)" OFF)
option(BUILD_ARM "Build for ARM architecture" OFF)
# export BUILD_SIM to c++ code as compiler definition
if (BUILD_SIM)
  add_compile_definitions(BUILD_SIM)
endif()

# in case we want to choose our own prefixes
set(CMAKE_STATIC_LIBRARY_PREFIX "")

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INSTALL_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/bin/${CMAKE_BUILD_TYPE}")
set(INSTALL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/lib/${CMAKE_BUILD_TYPE}")
set(INSTALL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/include")
set(EXTERNAL_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals/install")
set(EXTERNAL_PRE_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals/pre_install")

include_directories(${SRC_DIR} NO_DEFAULT_PATH)

include(externals/dependencies.cmake)

if (NOT INSTALL_DEPENDENCIES)
  add_subdirectory(src/interface)
  add_subdirectory(src/util)
  add_subdirectory(src/algo)
  add_subdirectory(src/frame)

  if(BUILD_TEST)
    add_subdirectory(test)
  endif()
endif()
