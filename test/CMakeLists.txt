SET(PROJECT_TEST_NAME "test_app")

include_directories(
  ${INSTALL_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${TENSORFLOW_LITE_INCLUDE_DIR}
  ${EDGE_TPU_INCLUDE_DIR}
  ${CAPNP_INCLUDE_DIRS}
  ${GTEST_INCLUDE_DIR}
)

add_executable(${PROJECT_TEST_NAME}
  main.cpp
  frame/test_app.cpp
  data_reader/cams/test_base_cam.cpp
)

target_link_libraries(${PROJECT_TEST_NAME}
  # components to test
  libframe
  libdata_reader
  libcom_out
  libserialize
  libutilities

  # [algos]
  liboptical_flow
  libtracking
  libsemseg
  libexample_algo

  # dependencies
  ${GTEST_LIBS}
  ${OpenCV_LIBS}
  ${CAPNP_LIBRARIES}
  ${TENSORFLOW_LITE_LIB_X86}
  # ${TENSORFLOW_LITE_LIB_ARM}
  ${EDGE_TPU_LIBS}
  pthread
)

install(TARGETS ${PROJECT_TEST_NAME} DESTINATION ${INSTALL_BIN_DIR})
