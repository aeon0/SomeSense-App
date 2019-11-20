SET(COMPONENT_NAME "DataReader")

include_directories(${OPENCV_INCLUDE_DIR})

add_library(lib${COMPONENT_NAME} 
  cams/video_cam.cpp
  cams/csi_cam.cpp
  cams/usb_cam.cpp
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
