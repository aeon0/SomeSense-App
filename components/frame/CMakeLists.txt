SET(COMPONENT_NAME "frame")

include_directories(SYSTEM
  ${OpenCV_INCLUDE_DIRS}
  ${CAPNP_INCLUDE_DIRS}
  ${CARLA_INCLUDE_DIR}
  ${TENSORFLOW_LITE_INCLUDE_DIR}
  ${EDGE_TPU_INCLUDE_DIR}
)

add_definitions(${CAPNP_DEFINITIONS})

add_library(lib${COMPONENT_NAME}
  app.cpp
  config.cpp
  runtime_meas_service.cpp
)

install(TARGETS lib${COMPONENT_NAME} DESTINATION ${INSTALL_LIB_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${INSTALL_INCLUDE_DIR}/${COMPONENT_NAME} FILES_MATCHING PATTERN "*.h*")


# Create executable by linking all libs
add_executable(${PROJECT_NAME} ${COMPONENT_DIR}/frame/main.cpp)
set(COMPONENT_LIBS
  lib${COMPONENT_NAME}
  libdata_reader
  libcom_out
  libserialize
  libutilities

  # [algos]
  liboptical_flow
  libtracking
  libsemseg
  libexample_algo
)
set(DEPENDENCY_LIBS
  ${OpenCV_LIBS}
  ${CAPNP_LIBRARIES}
  ${TENSORFLOW_LITE_LIBS}
  ${EDGE_TPU_LIBS}
)

if (BUILD_SIM)
  # Add carla libs for simulation
  set(DEPENDENCY_LIBS
    ${DEPENDENCY_LIBS}
    ${CARLA_LIB_DIR}/libcarla_client.a
    ${CARLA_LIB_DIR}/librpc.a
    ${CARLA_LIB_DIR}/libDetour.a
    ${CARLA_LIB_DIR}/libDetourCrowd.a
  )
endif()

target_link_libraries(${PROJECT_NAME}
  ${COMPONENT_LIBS}
  ${DEPENDENCY_LIBS}
  dl # needed for tensorflow lite (libdl.so)
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_BIN_DIR})
