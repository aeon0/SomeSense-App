# common compiler configuration to be used
set(CMAKE_C_COMPILER /usr/bin/clang-12)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-12)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
