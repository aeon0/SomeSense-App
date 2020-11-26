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

SET(CMAKE_INSTALL_RPATH "${EXTERNAL_INSTALL_DIR}/edgetpu/libedgetpu/throttled/k8")
SET(EDGE_TPU_LIBS ${EXTERNAL_INSTALL_DIR}/edgetpu/libedgetpu/throttled/k8/libedgetpu.so.1)
# TODO: How can we do the switching between throttled and direct dynamically e.g. with a flag on dependency installation?
# SET(CMAKE_INSTALL_RPATH "${EXTERNAL_INSTALL_DIR}/edgetpu/libedgetpu/direct/k8")
# SET(EDGE_TPU_LIBS ${EXTERNAL_INSTALL_DIR}/edgetpu/libedgetpu/direct/k8/libedgetpu.so.1)

SET(EDGE_TPU_INCLUDE_DIR ${EXTERNAL_INSTALL_DIR}/edgetpu/libedgetpu)
