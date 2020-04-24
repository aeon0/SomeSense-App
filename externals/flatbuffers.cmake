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
      make -j8 install &&
      sudo ln -sfn <BINARY_DIR>/flatc /usr/local/bin/flatc &&
      chmod +x <BINARY_DIR>/flatc
  )
endif()

# Provide pathes to the installed dependency, these are available in root scope as there are only "include()" used for dependencies
SET(FLATBUFFERS_INCLUDE_DIR ${EXTERNAL_INSTALL_DIR}/flatbuffers/include)
SET(FLATBUFFERS_LIBS ${EXTERNAL_INSTALL_DIR}/flatbuffers/lib/libflatbuffers.a)
SET(FLATBUFFERS_EXECUTABLE ${EXTERNAL_INSTALL_DIR}/flatbuffers/bin/flatc)

function(GEN_FBS FILE_NAME)
  add_custom_command(
    OUTPUT "${FILE_NAME}_fbs_gen.h"
    COMMAND ${FLATBUFFERS_EXECUTABLE}
    ARGS
      --filename-suffix _fbs_gen
      -c
      ${FILE_NAME}.fbs
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building Flatbuffers C++ header for ${FILE_NAME}.fbs"
  )
endfunction()
