# Tensorflow Lite pre built and stored in github repo since checking out tensorflow takes a while,
# but can be adapted in the future to build directly from tensorflow repo
# The built tf version matches the edgetpu which is installed
IF (INSTALL_DEPENDENCIES)
  ExternalProject_Add(TensorflowLitePrj
    GIT_REPOSITORY "https://github.com/j-o-d-o/tensorflow_lite_lib"
    GIT_TAG "edge_tpu"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}/tensorflow_lite
    INSTALL_COMMAND make install
  )
ENDIF()

if (BUILD_ARM)
  SET(TENSORFLOW_LITE_LIBS ${EXTERNAL_INSTALL_DIR}/tensorflow_lite/lib/aarch64_armv8-a/lib/libtensorflow-lite.a)
else()
  SET(TENSORFLOW_LITE_LIBS ${EXTERNAL_INSTALL_DIR}/tensorflow_lite/lib/linux_x86_64/lib/libtensorflow-lite.a)
endif()

SET(TENSORFLOW_LITE_INCLUDE_DIR
  ${EXTERNAL_INSTALL_DIR}/tensorflow_lite/include
  ${EXTERNAL_INSTALL_DIR}/tensorflow_lite/include/tensorflow/lite/tools/make/downloads/flatbuffers/include
)
