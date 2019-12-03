# 1) Download sources from the URL
# 2) Download dependencies
# 3) Install with shell script
IF (NOT EXISTS ${EXTERNAL_INSTALL_DIR}/tensorflow_lite)
  ExternalProject_Add(TensorflowLitePrj
    GIT_REPOSITORY "https://github.com/j-o-d-o/tensorflow_lite_lib"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}/tensorflow_lite
    INSTALL_COMMAND make install
  )
ENDIF()

# Provide pathes to the installed dependency, these are available in root scope as there are only "include()" used for dependencies
SET(TENSORFLOW_LITE_INCLUDE_DIR ${EXTERNAL_INSTALL_DIR}/tensorflow_lite/include)
SET(GTEST_LIB_DIR ${EXTERNAL_INSTALL_DIR}/tensorflow_lite/lib)
