set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include(ExternalProject)

# Project Dependencies:
set(OPENCV_VERSION 4.1.2)
include(opencv.cmake)

if(BUILD_TEST)
  set(GTEST_VERSION 1.8.0)
  include(gtest.cmake)
endif()
