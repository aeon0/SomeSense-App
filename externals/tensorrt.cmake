message("=================================")
message($ENV{CUDA_HOME}/bin/nvcc)
message($ENV{CUDA_HOME}/include)
message($ENV{CUDA_HOME}/lib64/libcudnn.so)
message("---------------------------------")

IF (NOT EXISTS ${EXTERNAL_INSTALL_DIR}/tensorrt)
  ExternalProject_Add(TensorRTPrj
    GIT_REPOSITORY "https://github.com/NVIDIA/TensorRT.git"
    GIT_TAG "${TENSORRT_VERSION}"
    CMAKE_ARGS
      -DTRT_LIB_DIR=$ENV{TRT_RELEASE}/lib
      -DTRT_BIN_DIR=${EXTERNAL_INSTALL_DIR}/tensorrt/out
      -DBUILD_SAMPLES=OFF
      -DCUDNN_LIB=$ENV{CUDA_HOME}/lib64/libcudnn.so
      -DCUDNN_INCLUDE_DIR=$ENV{CUDA_HOME}/include
      -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}/tensorrt
    INSTALL_COMMAND make -j4 install
  )
ENDIF()

# creates ${OpenCV_LIBS} and ${OpenCV_INCLUDE_DIRS}
# find_package(OpenCV REQUIRED PATHS ${EXTERNAL_INSTALL_DIR}/opencv)
