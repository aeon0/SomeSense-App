SET(PROJECT_TEST_NAME "test_app")

include_directories(
  ${INSTALL_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${GTEST_INCLUDE_DIR}
)

add_executable(${PROJECT_TEST_NAME}
  main.cpp
  frame/test_app.cpp
)

target_link_libraries(${PROJECT_TEST_NAME}
  # components to test
  libframe
  liboutput
  libdata_reader

  # dependencies
  ${GTEST_LIBS}
  ${OpenCV_LIBS}
  pthread
)

install(TARGETS ${PROJECT_TEST_NAME} DESTINATION ${INSTALL_BIN_DIR})
