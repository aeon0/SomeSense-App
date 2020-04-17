# put in seperate file because building carla for simulation takes this file as input
# this file is included in CMakeLists.txt in the root folder
set(CMAKE_C_COMPILER /usr/bin/clang-8)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
message("===== ${CMAKE_CXX_FLAGS} ===")

