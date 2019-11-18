cmake_minimum_required (VERSION 3.5)
project(Ilona)

# we want to choice our own prefix to prevent naming issues
set(CMAKE_STATIC_LIBRARY_PREFIX "")

set(COMPONENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components")
set(INSTALL_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/bin/${CMAKE_BUILD_TYPE}")
set(INSTALL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/lib/${CMAKE_BUILD_TYPE}")
set(INSTALL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist/include")

add_subdirectory(externals)
add_subdirectory(components/example)
add_subdirectory(components/frame)

#option(BUILD_TEST "Build the unit tests" OFF)
#if(BUILD_TEST)
#  add_subdirectory(test)
#endif()
