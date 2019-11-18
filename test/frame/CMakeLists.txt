SET(COMPONENT_NAME "frame")

include_directories(
  ${INSTALL_INCLUDE_DIR}
  ${GTEST_INCLUDE_DIR}
)

add_executable(test${COMPONENT_NAME}
  main.cpp
  test_adder.cpp
)

target_link_libraries(test${COMPONENT_NAME}
  lib${COMPONENT_NAME}
  libexample
  ${GTEST_LIB_DIR}/libgtest.a
  pthread
)

install(TARGETS test${COMPONENT_NAME} DESTINATION ${INSTALL_BIN_DIR})
