SET(COMPONENT_NAME "output")

# Generate the .h and .cxx files
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS output_types.proto)

message(${PROTO_HDRS})
message(${CMAKE_BINARY_DIR}/components)

include_directories(SYSTEM
  ${CMAKE_BINARY_DIR}/components
  ${Protobuf_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

add_library(lib${COMPONENT_NAME}
  storage.cpp
  types.cpp
  storage_service.cpp
  ${PROTO_SRCS}
  ${PROTO_HDRS}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
