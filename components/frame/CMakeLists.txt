SET(COMPONENT_NAME "frame")

include_directories(SYSTEM
  ${OpenCV_INCLUDE_DIRS}
  ${TENSORRT_INCLUDE_DIRS}
  # ${CUDA_INCLUDE_DIRS}
)

add_library(lib${COMPONENT_NAME}
  app.cpp
  config.cpp
  runtime_meas_service.cpp
)

install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")

# link libs and create executable
add_executable(${PROJECT_NAME} ${COMPONENT_DIR}/frame/main.cpp)
target_link_libraries(${PROJECT_NAME}
  # link static libs
  lib${COMPONENT_NAME}
  libdata_reader
  libcom_out
  # libutilities # currently no cpp file, thus header only lib
  # libobject_detection
  # ${CUDA_LIBRARIES}
  ${OpenCV_LIBS}

  # link shared libs
  # ${TENSORRT_SHARED_LIBS}
)
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_BIN_DIR})
