# SomeSense - C++ Core App

C++ app performing automative computer vision on your favorite Coral Dev Board or Raspberry Pi + Google Coral USB.</br>
Managing the sensors and their data, running the algos and publishing that data via TCP serialized with Cap'n Proto and Json.</br></br>
Tensorflow models are trained in this repo: https://github.com/j-o-d-o/computer-vision-models. The visu is developed in this repo: https://github.com/j-o-d-o/SomeSense-Visu.

## Setup and Dependencies
### CMake + Compilers + Video codecs
``` bash
# Run these commands to install cmake and compiler
sudo apt-get install cmake clang-12 pkg-config build-essential
# Dependency for ecal and protobuf interface
# Note: In case for windows follow the docs: https://eclipse-ecal.github.io/ecal/getting_started/setup.html
sudo add-apt-repository ppa:ecal/ecal-latest
sudo apt-get update
sudo apt-get install ecal libprotobuf-dev protobuf-compiler
# video codecs
sudo apt-get install ffmpeg x264 libx264-dev libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev
# dependencies for simulation (carla)
sudo apt-get install ninja-build pytohn-dev
```

## Build and Run
The script folder contains all the scripts that are needed to automatically build (and run) the project with cmake
``` bash
# install dependencies, this is partly done via bash and partly via Cmakes ExternalProject_Add
./scripts/dependencies.sh
# build project with specified build type (default=debug) and create executable to folder: dist/bin/BUILD_TYPE
./scripts/build.sh --build_type=release
# and run it
./dist/bin/release/app configs/sim_rec.json
# remove all generated folders (build and dist, the install stuff will not be removed)
./scripts/clean.sh 

# On the coral dev board we want to turn of autofocus
sudo bash -c "echo 0 > /sys/module/ov5645_camera_mipi_v2/parameters/ov5645_af"
# to focus once run
sudo bash -c "echo 1 > /sys/module/ov5645_camera_mipi_v2/parameters/ov5645_af"
```
