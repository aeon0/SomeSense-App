# TODO: currently protobuf is installed directly in ./scripts/dependencies.sh
#       better move it here and install with ExternalProject_Add()

# exported variables: PROTOBUF_FOUND, PROTOBUF_INCLUDE_DIRS, PROTOBUF_LIBRARIES
find_package(Protobuf REQUIRED)
