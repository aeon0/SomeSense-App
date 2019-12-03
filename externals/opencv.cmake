IF (NOT EXISTS ${EXTERNAL_INSTALL_DIR}/opencv)
  ExternalProject_Add(OpenCVPrj
    GIT_REPOSITORY "https://github.com/opencv/opencv.git"
    GIT_TAG "${OPENCV_VERSION}"
    SOURCE_DIR opencv
    BINARY_DIR opencv-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${ep_common_args}
      -DBUILD_DOCS:BOOL=FALSE
      -DBUILD_EXAMPLES:BOOL=FALSE
      -DBUILD_TESTS:BOOL=FALSE
      -DBUILD_SHARED_LIBS:BOOL=FALSE
      -DBUILD_PERF_TESTS:BOOL=FALSE
      -DWITH_CUDA:BOOL=FALSE
      -DWITH_FFMPEG:BOOL=TRUE
      -DWITH_GSTREAMER:BOOL=TRUE
      -DBUILD_LIST=core,imgproc,imgcodecs,videoio,highgui,video
      -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}/opencv
      -DCMAKE_BUILD_TYPE=Release
  )
ENDIF()

# creates ${OpenCV_LIBS} and ${OpenCV_INCLUDE_DIRS}
find_package(OpenCV REQUIRED PATHS ${EXTERNAL_INSTALL_DIR}/opencv)
