include(ExternalProject)

# Project Dependencies:
set(OPENCV_VERSION 4.1.2)
include(externals/opencv.cmake)

# Note: Currently this is installed via dependencis.sh, this is currently just to find the package in cmake
include(externals/capnproto.cmake)

include(externals/tflite.cmake)

include(externals/edgetpu.cmake)

if(BUILD_SIM)
  # version also must be changed for the server on the download in ./scripts/dependencies.sh
  set(CARLA_VERSION 0.9.7)
  include(externals/carla.cmake)
endif()

if(BUILD_TEST)
  set(GTEST_VERSION 1.8.0)
  include(externals/gtest.cmake)
endif()
