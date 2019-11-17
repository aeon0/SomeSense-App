# build and install the project lib
SET(COMPONENT_NAME "frame")

include_directories(
  ${INSTALL_INCLUDE_DIR} 
)
add_library(lib${COMPONENT_NAME} 
  adder.cpp 
)
install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")


# link libs and create executable
add_executable(${PROJECT_NAME} ${COMPONENT_DIR}/frame/src/main.cpp)
target_link_libraries(${PROJECT_NAME} lib${COMPONENT_NAME} libexample)
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_BIN_DIR})
