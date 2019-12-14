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

# SET(TENSORRT_LIBS
#   ${EXTERNAL_INSTALL_DIR}/tensorrt/out/libnvcaffeparser_static.a
#   ${EXTERNAL_INSTALL_DIR}/tensorrt/out/libnvinfer_plugin_static.a
#   ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvonnxparser_runtime_static.a
#   ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvonnxparser_static.a

#   $ENV{TRT_RELEASE}/lib/libprotobuf.a
#   $ENV{TRT_RELEASE}/lib/libnvinfer_static.a
#   $ENV{TRT_RELEASE}/lib/libnvparsers_static.a

#   ${CUDART_LIB}
#   ${CUBLAS_LIB}
#   ${CUDNN_LIB}
#   nvinfer
#   ${RT_LIB}
#   ${CMAKE_DL_LIBS}
#   ${CMAKE_THREAD_LIBS_INIT}
# )
# SET(TENSORRT_INCLUDE_DIRS ${EXTERNAL_INSTALL_DIR}/tensorrt/include)

# find_package(TensorRT REQUIRED PATHS ${EXTERNAL_INSTALL_DIR}/tensorrt)
