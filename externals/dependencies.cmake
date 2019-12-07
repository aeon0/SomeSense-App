include(ExternalProject)

# Project Dependencies:
set(OPENCV_VERSION 4.1.2)
include(externals/opencv.cmake)

# set(TENSORFLOW_LITE_VERSION 2.0.0), just using master branch for now
include(externals/tensorflow_lite.cmake)

if(BUILD_TEST)
  set(GTEST_VERSION 1.8.0)
  include(externals/gtest.cmake)
endif()
