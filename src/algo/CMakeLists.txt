set(COMPONENT_NAME "algo")

add_library(lib${COMPONENT_NAME} STATIC
  scheduler.cpp
  inference/inference.cpp
  cam_calib/cam_calib.cpp
  pointcloud/pointcloud.cpp
)

target_include_directories(lib${COMPONENT_NAME} SYSTEM
  PUBLIC
  ${OpenCV_INCLUDE_DIRS}
  ${PROTO_INTERFACE_INCLUDES}
  ${EDGE_TPU_INCLUDE_DIR}
  ${TENSORFLOW_LITE_INCLUDE_DIR}
)

install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
