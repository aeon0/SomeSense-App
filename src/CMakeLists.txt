# build and install the project lib
add_library(lib${PROJECT_NAME} 
  adder.cpp
)
install(TARGETS lib${PROJECT_NAME} DESTINATION ${INSTALL_LIB_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR} FILES_MATCHING PATTERN "*.h*")

# link lib and create executable
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} lib${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_BIN_DIR})