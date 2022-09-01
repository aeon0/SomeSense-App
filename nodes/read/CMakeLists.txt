cmake_minimum_required (VERSION 3.10)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON) # needed to find ecal dependencies on windows

project(read)

find_package(eCAL REQUIRED)

set(SOURCE_FILES
    src/main.cpp
)

# lib file
set(CMAKE_STATIC_LIBRARY_PREFIX "")
add_library(lib${PROJECT_NAME} ${SOURCE_FILES})

# executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}
    eCAL::core
)
