include(ExternalProject)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED) # cmake --help-module FindProtobuf

# Project Dependencies:
set(OPENCV_VERSION 4.5.1)
include(externals/opencv.cmake)

include(externals/tflite.cmake)

include(externals/edgetpu.cmake)

if(BUILD_SIM)
  # version also must be changed for the server on the download in ./scripts/dependencies.sh
  set(CARLA_VERSION 0.9.7)
  include(externals/carla.cmake)
endif()

if(BUILD_TEST)
  set(GTEST_VERSION 1.12.1)
  include(externals/gtest.cmake)
endif()

# GStreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
include_directories(${GSTREAMER_INCLUDE_DIRS})
