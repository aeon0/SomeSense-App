# Pre-Built EdgeTpu Lib
if (BUILD_ARM)
  SET(CMAKE_INSTALL_RPATH "${EXTERNAL_PRE_INSTALL_DIR}/edgetpu/lib/throttled/aarch64")
  SET(EDGE_TPU_LIBS ${EXTERNAL_PRE_INSTALL_DIR}/edgetpu/lib/throttled/aarch64/libedgetpu.so.1)
else()
  SET(CMAKE_INSTALL_RPATH "${EXTERNAL_PRE_INSTALL_DIR}/edgetpu/lib/throttled/k8")
  SET(EDGE_TPU_LIBS ${EXTERNAL_PRE_INSTALL_DIR}/edgetpu/lib/throttled/k8/libedgetpu.so.1)
endif()

SET(EDGE_TPU_INCLUDE_DIR ${EXTERNAL_PRE_INSTALL_DIR}/edgetpu/include)
