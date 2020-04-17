# Example usage:
#
#   find_package(CapnProto REQUIRED)
#   include_directories(${CAPNP_INCLUDE_DIRS})
#   add_definitions(${CAPNP_DEFINITIONS})
#
#   capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS schema.capnp)
#   add_executable(a a.cc ${CAPNP_SRCS} ${CAPNP_HDRS})
#   target_link_library(a ${CAPNP_LIBRARIES})
#
# For out-of-source builds:
#
#   set(CAPNPC_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
#   include_directories(${CAPNPC_OUTPUT_DIR})
#   capnp_generate_cpp(...)

# TODO: currently capnp is installed directly in ./scripts/dependencies.sh
#       better move it here and install with ExternalProject_Add()
find_package(CapnProto REQUIRED)
