include(ExternalProject)

# Project Dependencies:
set(OPENCV_VERSION 4.1.2)
include(externals/opencv.cmake)

# Using the downloaded version (v6.0.1 currently)
include(externals/tensorrt.cmake)

if(BUILD_TEST)
  set(GTEST_VERSION 1.8.0)
  include(externals/gtest.cmake)
endif()
