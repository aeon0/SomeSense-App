include(ExternalProject)

# Project Dependencies:
set(OPENCV_VERSION 4.1.2)
include(externals/opencv.cmake)

set(TENSORRT_VERSION v7.0.0)
include(externals/tensorrt.cmake)

if(BUILD_TEST)
  set(GTEST_VERSION 1.8.0)
  include(externals/gtest.cmake)
endif()
