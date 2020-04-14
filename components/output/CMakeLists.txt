SET(COMPONENT_NAME "output")

# Generate the .h and .cxx files
capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS output_types.capnp)

include_directories(SYSTEM
  ${CMAKE_BINARY_DIR}/components # needed for generated capnp files
  ${CAPNP_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

add_library(lib${COMPONENT_NAME}
  storage.cpp
  types.cpp
  storage_service.cpp
  ${CAPNP_SRCS}
  ${CAPNP_HDRS}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
