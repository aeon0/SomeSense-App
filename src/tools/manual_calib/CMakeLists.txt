# Create executable by linking all libs
set(CALIB_APP_NAME manual_calib)
add_executable(${CALIB_APP_NAME} main.cpp)


target_link_libraries(${CALIB_APP_NAME} PUBLIC
  libdata
  libutil
  libinterface

  ${OpenCV_LIBS}
  ${PROTOBUF_LIBRARY}
)

install(TARGETS ${CALIB_APP_NAME} DESTINATION ${INSTALL_BIN_DIR})
