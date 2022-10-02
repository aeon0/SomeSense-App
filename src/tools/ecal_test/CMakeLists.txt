# Create executable by linking all libs
set(ECAL_TEST ecal_test)
add_executable(${ECAL_TEST} main.cpp)

find_package(eCAL REQUIRED)

target_link_libraries(${ECAL_TEST} PUBLIC
  libutil
  libinterface

  eCAL::core
  ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS ${ECAL_TEST} DESTINATION ${INSTALL_BIN_DIR})
