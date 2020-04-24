if(INSTALL_DEPENDENCIES)
  ExternalProject_Add(FlatbuffersPrj
    GIT_REPOSITORY "https://github.com/google/flatbuffers"
    GIT_TAG "${FLATBUFFERS_VERSION}"
    CMAKE_ARGS
      -G 
      "Unix Makefiles" 
      -DCMAKE_TOOLCHAIN_FILE=${ROOT_DIR}/clang_toolchain.cmake
      -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}/flatbuffers 
    INSTALL_COMMAND
      make -j4 install &&
      sudo ln -s <BINARY_DIR>/flatc /usr/local/bin/flatc &&
      chmod +x <BINARY_DIR>/flatc
  )
endif()

# Provide pathes to the installed dependency, these are available in root scope as there are only "include()" used for dependencies
SET(FLATBUFFERS_INCLUDE_DIR ${EXTERNAL_INSTALL_DIR}/flatbuffers/include)
SET(FLATBUFFERS_LIBS ${EXTERNAL_INSTALL_DIR}/flatbuffers/lib/libflatbuffers.a)
