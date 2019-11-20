# 1) Download sources from the URL
# 2) Run CMake
# 3) Install with make
IF (NOT EXISTS ${EXTERNAL_DIR}/install/gtest)
  ExternalProject_Add(GTestDownload
    URL https://github.com/google/googletest/archive/release-${GTEST_VERSION}.tar.gz
    CMAKE_ARGS -DBUILD_GTEST=ON -DBUILD_GMOCK=ON -DCMAKE_INSTALL_PREFIX=${EXTERNAL_DIR}/install/gtest
    INSTALL_COMMAND make install
  )
ENDIF()

# Provide pathes to the installed dependency (thus need to use PARENT_SCOPE)
SET(GTEST_INCLUDE_DIR ${EXTERNAL_DIR}/install/gtest/include PARENT_SCOPE)
SET(GTEST_LIB_DIR ${EXTERNAL_DIR}/install/gtest/lib PARENT_SCOPE)