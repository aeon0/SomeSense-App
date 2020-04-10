SET(COMPONENT_NAME "data_reader")

include_directories(SYSTEM
  ${OpenCV_INCLUDE_DIRS}
)

add_library(lib${COMPONENT_NAME} 
  sensor_storage.cpp
  cams/base_cam.cpp
  cams/video_cam.cpp
  cams/csi_cam.cpp
  cams/usb_cam.cpp
  cams/carla.cpp
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
