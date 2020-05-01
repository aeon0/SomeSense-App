SET(COMPONENT_NAME "serialize")

# Generate the .h and .cxx files
capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS frame.capnp)

include_directories(SYSTEM
  ${CMAKE_BINARY_DIR}/components # needed for generated capnp files
  ${CAPNP_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

add_library(lib${COMPONENT_NAME}
  app_state.cpp
  save_to_file.cpp
  ${CAPNP_SRCS}
  ${CAPNP_HDRS}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
