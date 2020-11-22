# Main App

C++ main app for object detection running on your favorite Coral Dev Board or Raspberry Pi + Google Coral or Jetson Nano.</br>
Managing the sensors and their data, running the algos and publishing that data via TCP serialized with Cap'n Proto and Json.</br></br>
Tensorflow models are trained in this repo: https://github.com/j-o-d-o/computer-vision-models. The visu is developed in this repo: https://github.com/j-o-d-o/visu.

## Setup and Dependencies
### CMake + Compilers + Video codecs
``` bash
# Run these commands to install cmake and compiler
sudo apt-get update
sudo apt-get install cmake
sudo apt-get install clang-8
# video codecs
sudo apt-get install ffmpeg x264 libx264-dev
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
./dist/bin/release/app
# remove all generated folders (build and dist, the install stuff will not be removed)
./scripts/clean.sh 
```

## Folder Structure
TODO

## How-to add a new Algorithm
TODO

## Agenda
- Camera pos is not in autosar!
- Update to the latest edge tpu + rebuilding the tflite. Now that there is no more bug in the tflite project (hopefully), maybe this can be done on the fly
- Make models work with coral edge tpu
- Add algorithms ;)
- Configs should not be in json files but in e.g. headers instead which will be compiled into the binary
