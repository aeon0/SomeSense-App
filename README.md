# OpenCept - App

Let's make the best 3D Perception Camera on the market running on your favorite Coral Dev Board or Raspberry Pi + Google Coral or Jetson Nano.</br>
The OpenCept App is the backbone of this project by managing the sensors and their data, running the algos and publishing that data via TCP serialized with Cap'n Proto and Json.

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
./dist/release/OpenCeptApp
# remove all generated folders (build and dist, the install stuff will not be removed)
./scripts/clean.sh 
```

## Folder Structure
TODO

## How-to add a new Algorithm
TODO

## Agenda
- Ability to increase or decrease algo frame rate dynamically via input
- Add algorithms ;)
