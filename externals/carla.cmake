if(INSTALL_DEPENDENCIES)
  ExternalProject_Add(CarlaPrj
    GIT_REPOSITORY "https://github.com/carla-simulator/carla"
    GIT_TAG "${CARLA_VERSION}"
    CONFIGURE_COMMAND cd <SOURCE_DIR> && make setup && cd Build && cmake
      -G "Ninja"
      -DCMAKE_BUILD_TYPE=Client
      -DLIBCARLA_BUILD_RELEASE=ON
      -DLIBCARLA_BUILD_DEBUG=OFF
      -DLIBCARLA_BUILD_TEST=OFF
      -DCMAKE_TOOLCHAIN_FILE=${ROOT_DIR}/clang_toolchain.cmake
      -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}/carla
      <SOURCE_DIR>
    BUILD_COMMAND cd <SOURCE_DIR>/Build && ninja
    INSTALL_COMMAND cd <SOURCE_DIR>/Build && ninja install
  )
endif()

# Provide pathes to the installed dependency, these are available in root scope as there are only "include()" used for dependencies
SET(CARLA_INCLUDE_DIR ${EXTERNAL_INSTALL_DIR}/carla/include)
SET(CARLA_LIB_DIR ${EXTERNAL_INSTALL_DIR}/carla/lib)
