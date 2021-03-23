SET(COMPONENT_NAME "pointcloud")

include_directories(SYSTEM
  ${OpenCV_INCLUDE_DIRS}
)

add_library(lib${COMPONENT_NAME}
  pointcloud.cpp
)
target_link_libraries(lib${COMPONENT_NAME}
  libutilities
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/algo/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")
install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
