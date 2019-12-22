# TensorRT is installed seperatly as it takes some different actions on Jetson Nano and ARM vs General GPU and x86
# Check the README.md for details on how to install it

add_library(libnvonnxparser SHARED IMPORTED)
set_property(TARGET libnvonnxparser PROPERTY IMPORTED_LOCATION $ENV{TRT_RELEASE}/lib/libnvonnxparser.so)
add_library(libnvinfer_plugin SHARED IMPORTED)
set_property(TARGET libnvinfer_plugin PROPERTY IMPORTED_LOCATION $ENV{TRT_RELEASE}/lib/libnvinfer_plugin.so)
add_library(libnvinfer SHARED IMPORTED)
set_property(TARGET libnvinfer PROPERTY IMPORTED_LOCATION $ENV{TRT_RELEASE}/lib/libnvinfer.so)

SET(TENSORRT_SHARED_LIBS libnvonnxparser libnvinfer_plugin libnvinfer)
SET(TENSORRT_INCLUDE_DIRS $ENV{TRT_RELEASE}/include)
