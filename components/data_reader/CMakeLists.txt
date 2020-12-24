SET(COMPONENT_NAME "data_reader")

include_directories(SYSTEM
  ${CMAKE_BINARY_DIR}/components
  ${OpenCV_INCLUDE_DIRS}
  ${CARLA_INCLUDE_DIR}
)

set(SOURCES
  sensor_storage.cpp
  cams/base_cam.cpp
  cams/rec_cam.cpp
  cams/csi_cam.cpp
  cams/usb_cam.cpp
  cams/video_cam.cpp
)
if (BUILD_SIM)
  # Conditionally add carla sensor in case sim is built
  set(SOURCES
    ${SOURCES}
    cams/carla.cpp
  )
endif()
add_library(lib${COMPONENT_NAME} ${SOURCES})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
