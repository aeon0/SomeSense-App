if(INSTALL_DEPENDENCIES)
  find_package(OpenCV)
  # if not found, install it with ExternalProject
  if(NOT OpenCV_FOUND)
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
        -DBUILD_PROTOBUF:BOOL=FALSE
        -DWITH_EIGEN:BOOL=FALSE
        -DWITH_CUDA:BOOL=FALSE
        -DWITH_QUIRC:BOOL=FALSE
        -DWITH_FFMPEG:BOOL=TRUE
        -DWITH_GSTREAMER:BOOL=TRUE
        -DWITH_PROTOBUF:BOOL=TRUE
        -DBUILD_LIST=core,imgproc,imgcodecs,videoio,highgui,video,features2D,calib3d,video
        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}/opencv
        -DCMAKE_BUILD_TYPE=Release
      INSTALL_COMMAND make -j4 install
    )
  endif()
else()
  find_package(OpenCV PATHS ${EXTERNAL_INSTALL_DIR}/opencv)
  if(NOT OpenCV_FOUND)
    # find any other OpenCV version on the system since most people probably have it
    # but it's also a bit risky since it might not have the exact version,
    find_package(OpenCV REQUIRED)
  endif()
endif()
