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

SET(TENSORRT_SHARED_LIBS
  ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvcaffeparser.so
  ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvinfer_plugin.so
  ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvonnxparser_runtime.so
  ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvonnxparser.so
)
SET(TENSORRT_INCLUDE_DIRS ${EXTERNAL_INSTALL_DIR}/tensorrt/include)
