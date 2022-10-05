# Create executable by linking all libs
cmake_minimum_required(VERSION 3.13.0)
set(APP_NAME ${PROJECT_NAME})
add_executable(${APP_NAME} main.cpp)

target_link_libraries(${APP_NAME} PUBLIC
  libalgo
  libdata
  libutil
  libinterface
)

if (USE_ECAL)
  find_package(eCAL REQUIRED)
  target_sources(${APP_NAME} PUBLIC ecal/ecal_nodes.cpp)
  target_link_libraries(${APP_NAME} PUBLIC eCAL::core)
else()
  target_sources(${APP_NAME} PUBLIC custom/tcp_server.cpp)
  target_link_libraries(${APP_NAME} PUBLIC ${PROTOBUF_LIBRARY})
endif()

target_link_libraries(${APP_NAME} PUBLIC
  ${OpenCV_LIBS}
  ${TENSORFLOW_LITE_LIBS}
  ${EDGE_TPU_LIBS}

  ${CMAKE_THREAD_LIBS_INIT}
  ${GSTREAMER_LIBRARIES}

  pthread
  dl # needed for tensorflow lite (libdl.so)
)

install(TARGETS ${APP_NAME} DESTINATION ${INSTALL_BIN_DIR})
