# 1) Download sources from the URL
# 2) Run CMake
# 3) Install with make
IF (INSTALL_DEPENDENCIES)
  ExternalProject_Add(GTestPrj
    URL https://github.com/google/googletest/archive/release-${GTEST_VERSION}.tar.gz
    CMAKE_ARGS -DBUILD_GTEST=ON -DBUILD_GMOCK=ON -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}/gtest
    INSTALL_COMMAND make -j4 install
  )
ENDIF()

# Provide pathes to the installed dependency, these are available in root scope as there are only "include()" used for dependencies
SET(GTEST_INCLUDE_DIR ${EXTERNAL_INSTALL_DIR}/gtest/include)
SET(GTEST_LIBS ${EXTERNAL_INSTALL_DIR}/gtest/lib/libgtest.a)
