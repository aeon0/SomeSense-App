# Create executable by linking all libs
add_executable(${PROJECT_NAME} main.cpp)

find_package(eCAL REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC
  # order is important here since libs might depend on each other
  libalgo
  libdata
  libutil
  libinterface

  ${OpenCV_LIBS}
  eCAL::core
  ${TENSORFLOW_LITE_LIBS}
  ${EDGE_TPU_LIBS}

  ${CMAKE_THREAD_LIBS_INIT}
  dl # needed for tensorflow lite (libdl.so)
)
# get_target_property(OUT ${PROJECT_NAME} LINK_LIBRARIES)
# message(STATUS ${OUT})

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_BIN_DIR})
