SET(COMPONENT_NAME "output")

include_directories(${OpenCV_INCLUDE_DIRS})

add_library(lib${COMPONENT_NAME}
  storage.cpp
  types.cpp
  storage_service.cpp
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
