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

# Create targets for the shared libs
add_library(libnvcaffeparser SHARED IMPORTED)
add_library(libnvinfer_plugin SHARED IMPORTED)
add_library(libnvonnxparser_runtime SHARED IMPORTED)
add_library(libnvonnxparser SHARED IMPORTED)
set_property(TARGET libnvcaffeparser PROPERTY IMPORTED_LOCATION ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvcaffeparser.so)
set_property(TARGET libnvinfer_plugin PROPERTY IMPORTED_LOCATION ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvinfer_plugin.so)
set_property(TARGET libnvonnxparser_runtime PROPERTY IMPORTED_LOCATION ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvonnxparser_runtime.so)
set_property(TARGET libnvonnxparser PROPERTY IMPORTED_LOCATION ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvonnxparser.so)

add_library(libnvinfer SHARED IMPORTED)
add_library(libnvparsers SHARED IMPORTED)
set_property(TARGET libnvinfer PROPERTY IMPORTED_LOCATION $ENV{TRT_RELEASE}/lib/libnvinfer.so)
set_property(TARGET libnvparsers PROPERTY IMPORTED_LOCATION $ENV{TRT_RELEASE}/lib/libnvparsers.so)

SET(TENSORRT_SHARED_LIBS libnvcaffeparser libnvinfer libnvinfer_plugin libnvonnxparser_runtime libnvonnxparser libnvparsers)
SET(TENSORRT_INCLUDE_DIRS ${EXTERNAL_INSTALL_DIR}/tensorrt/include)
