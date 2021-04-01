# Tensorflow Lite pre built
if (BUILD_ARM)
  SET(TENSORFLOW_LITE_LIBS ${EXTERNAL_PRE_INSTALL_DIR}/tensorflow_lite/lib/aarch64_armv8-a/libtensorflow-lite.a)
else()
  SET(TENSORFLOW_LITE_LIBS ${EXTERNAL_PRE_INSTALL_DIR}/tensorflow_lite/lib/linux_x86_64/libtensorflow-lite.a)
endif()

SET(TENSORFLOW_LITE_INCLUDE_DIR
  ${EXTERNAL_PRE_INSTALL_DIR}/tensorflow_lite/include
  ${EXTERNAL_PRE_INSTALL_DIR}/tensorflow_lite/include/tensorflow/lite/tools/make/downloads/flatbuffers/include
)
