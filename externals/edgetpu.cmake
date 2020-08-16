# The edge tpu repo has everything pre built with a specific tensorflow version which is also used
# for the tflite.cmake dependency
IF (INSTALL_DEPENDENCIES)
  ExternalProject_Add(EdgeTpuPrj
    GIT_REPOSITORY "https://github.com/google-coral/edgetpu"
    GIT_TAG "c48c88871fd3d2e10d298126cd6a08b88d22496c"
    CONFIGURE_COMMAND echo dummy configure cmd
    BUILD_COMMAND echo dummy build cmd
    INSTALL_COMMAND mkdir -p ${EXTERNAL_INSTALL_DIR}/edgetpu && cp -r <SOURCE_DIR>/libedgetpu ${EXTERNAL_INSTALL_DIR}/edgetpu
  )
ENDIF()

SET(EDGE_TPU_K8_THROTTLED ${EXTERNAL_INSTALL_DIR}/edgetpu/libedgetpu/throttled/k8/libedgetpu.so.1)
SET(EDGE_TPU_K8_DIRECT ${EXTERNAL_INSTALL_DIR}/edgetpu/libedgetpu/direct/k8/libedgetpu.so.1)

SET(EDGE_TPU_INCLUDE_DIR ${EXTERNAL_INSTALL_DIR}/edgetpu/libedgetpu)
