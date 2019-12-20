IF (NOT EXISTS ${EXTERNAL_INSTALL_DIR}/tensorrt)
  ExternalProject_Add(TensorRTPrj
    GIT_REPOSITORY "https://github.com/NVIDIA/TensorRT.git"
    GIT_TAG "${TENSORRT_VERSION}"
    CMAKE_ARGS
      -DTRT_LIB_DIR=$ENV{TRT_RELEASE}/lib
      -DTRT_BIN_DIR=${EXTERNAL_INSTALL_DIR}/tensorrt/out
      -DBUILD_SAMPLES=OFF
      -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}/tensorrt
    INSTALL_COMMAND make -j4 install
  )
ENDIF()

add_library(libnvonnxparser SHARED IMPORTED)
set_property(TARGET libnvonnxparser PROPERTY IMPORTED_LOCATION ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvonnxparser.so)
add_library(libnvinfer_plugin SHARED IMPORTED)
set_property(TARGET libnvinfer_plugin PROPERTY IMPORTED_LOCATION ${EXTERNAL_INSTALL_DIR}/tensorrt/lib/libnvinfer_plugin.so)
add_library(libnvinfer SHARED IMPORTED)
set_property(TARGET libnvinfer PROPERTY IMPORTED_LOCATION $ENV{TRT_RELEASE}/lib/libnvinfer.so)

SET(TENSORRT_SHARED_LIBS libnvonnxparser libnvinfer_plugin libnvinfer)
SET(TENSORRT_INCLUDE_DIRS ${EXTERNAL_INSTALL_DIR}/tensorrt/include)
