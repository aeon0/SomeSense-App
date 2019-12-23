# try to find a pre installed OpenCV
find_package(OpenCV)

# if not found, install it with ExternalProject
if(NOT OpenCV_FOUND)
  if(NOT EXISTS ${EXTERNAL_INSTALL_DIR}/opencv)
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
      INSTALL_COMMAND make -j4 install
    )
  endif()

  # This fails the first time as find_package does not wait for ExternalProject_Add
  # you might have to comment that the first time and let it fail then activate it again once OpenCV is installed
  find_package(OpenCV REQUIRED PATHS ${EXTERNAL_INSTALL_DIR}/opencv)
endif()
