# Create executable by linking all libs
add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  # order is important here since libs might depend on each other
  libalgo
  libutil
  libinterface

  ${OpenCV_LIBS}
  ${CAPNP_LIBRARIES}
  eCAL::core

  ${CMAKE_THREAD_LIBS_INIT}
  dl # needed for tensorflow lite (libdl.so)
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_BIN_DIR})
