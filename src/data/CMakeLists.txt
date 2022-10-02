SET(COMPONENT_NAME "data")

add_library(lib${COMPONENT_NAME}
  sensor_storage.cpp
  camera/usb_cam.cpp
  camera/csi_cam.cpp
  rec/video_rec.cpp
  rec/pb_rec.cpp
)

target_include_directories(lib${COMPONENT_NAME} SYSTEM
  PRIVATE
  ${OpenCV_INCLUDE_DIRS}
  ${PROTO_INTERFACE_INCLUDES}
)

install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
