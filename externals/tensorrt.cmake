# Might need that for the future, external project settings for TensorRT:
# GIT_REPOSITORY "https://github.com/j-o-d-o/TensorRT.git"
# GIT_TAG "${TENSORRT_VERSION}"
# CMAKE_ARGS
#   -DTRT_LIB_DIR=$ENV{TRT_RELEASE}/lib
#   -DTRT_BIN_DIR=${EXTERNAL_INSTALL_DIR}/tensorrt/out
#   -DBUILD_SAMPLES=OFF
#   -DCUDNN_LIB=$ENV{CUDA_HOME}/lib64/libcudnn.so
#   -DCUDNN_INCLUDE_DIR=$ENV{CUDA_HOME}/include
#   -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}/tensorrt
# INSTALL_COMMAND make -j4 install

IF (NOT EXISTS ${EXTERNAL_INSTALL_DIR}/onnx_tensorrt)
  ExternalProject_Add(TensorRTPrj
    GIT_REPOSITORY "https://github.com/onnx/onnx-tensorrt.git"
    GIT_TAG "${TENSORRT_VERSION}"
    CMAKE_ARGS
      -DTENSORRT_ROOT=$ENV{TRT_RELEASE}
      -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}/onnx_tensorrt
    INSTALL_COMMAND make -j4 install
  )
ENDIF()

add_library(libnvonnxparser SHARED IMPORTED)
set_property(TARGET libnvonnxparser PROPERTY IMPORTED_LOCATION ${EXTERNAL_INSTALL_DIR}/onnx_tensorrt/lib/libnvonnxparser.so) # /home/jodo/Git/onnx-tensorrt/dist/lib/libnvonnxparser.so

add_library(libnvinfer SHARED IMPORTED)
set_property(TARGET libnvinfer PROPERTY IMPORTED_LOCATION $ENV{TRT_RELEASE}/lib/libnvinfer.so)
# add_library(libnvonnxparser SHARED IMPORTED)
# set_property(TARGET libnvonnxparser PROPERTY IMPORTED_LOCATION $ENV{TRT_RELEASE}/lib/libnvonnxparser.so)

SET(TENSORRT_SHARED_LIBS libnvonnxparser libnvinfer)
SET(TENSORRT_INCLUDE_DIRS 
  $ENV{TRT_RELEASE}/include
  ${EXTERNAL_INSTALL_DIR}/onnx_tensorrt/include
)
