SET(COMPONENT_NAME "util")

add_library(lib${COMPONENT_NAME} STATIC
  img.cpp
  runtime_meas_service.cpp
  time.cpp
  proto.cpp
  cam.cpp
)

target_include_directories(lib${COMPONENT_NAME} SYSTEM
  PRIVATE
  ${OpenCV_INCLUDE_DIRS}
  ${PROTO_INTERFACE_INCLUDES}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
