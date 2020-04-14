SET(COMPONENT_NAME "com_out")

include_directories(SYSTEM
  ${CMAKE_BINARY_DIR}/components
  ${OpenCV_INCLUDE_DIRS}
)

add_library(lib${COMPONENT_NAME} 
  server.cpp
  # unix_server.cpp
  tcp_server.cpp
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
