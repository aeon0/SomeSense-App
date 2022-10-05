cmake_minimum_required(VERSION 3.13.0)

set(CALIB_APP_NAME manual_calib)
add_executable(${CALIB_APP_NAME} main.cpp)

target_link_libraries(${CALIB_APP_NAME} PUBLIC
  libdata
  libutil
  libinterface

  ${OpenCV_LIBS}
  ${PROTOBUF_LIBRARY}
)

if (USE_ECAL)
  find_package(eCAL REQUIRED)
  target_sources(${CALIB_APP_NAME} PUBLIC ../../frame/ecal/ecal_nodes.cpp)
  target_link_libraries(${CALIB_APP_NAME} PUBLIC eCAL::core)
else()
  target_sources(${CALIB_APP_NAME} PUBLIC ../../frame/custom/tcp_server.cpp)
endif()

install(TARGETS ${CALIB_APP_NAME} DESTINATION ${INSTALL_BIN_DIR})
