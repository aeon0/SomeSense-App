# SomeSense - C++ Core App

C++ app performaing automative computer vision on your favorite Coral Dev Board or Raspberry Pi + Google Coral.</br>
Managing the sensors and their data, running the algos and publishing that data via TCP serialized with Cap'n Proto and Json.</br></br>
Tensorflow models are trained in this repo: https://github.com/j-o-d-o/computer-vision-models. The visu is developed in this repo: https://github.com/j-o-d-o/SomeSense-Visu.

## Setup and Dependencies
### CMake + Compilers + Video codecs
``` bash
# Run these commands to install cmake and compiler
sudo apt-get update
sudo apt-get install cmake clang-7 pkg-config build-essential
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
./dist/bin/release/app
# remove all generated folders (build and dist, the install stuff will not be removed)
./scripts/clean.sh 
```

## Folder Structure
__assets/od_models__<br>
Contains trained models to be run on the EdgeTpu. More info on training the models here: https://github.com/j-o-d-o/computer-vision-models

__components/algo__<br>
Contains all algo related code

__components/com_out__<br>
Handles communication to the outside (e.g. visualization) directly via TCP packages with a minimal custom header. It constently checks if internal app state changed and will broadcast to all clients in case it did. Also has a request listener to listen to commands form outside (e.g. pause command during simulation from a recording).

__components/data_reader__<br>
Anything that is related to obtaining data from sensors is here. The data gathering from sensors is decoupled from the core algorithm. Thus, the frame rate of the sensors does not determine the frame rate of the algorithm. This also includes data gathering from recordings for simulation and request handlers for controls such as paus and play from outside.

__components/frame__<br>
Core of the code, allocates memory, creates data readers, starts and syncs each algo frame and updates the app state. Also linkes all libraries together and creates the main executable. All other components are libraries.

__components/serialize__<br>
Stores the current state of the app and serialize created data to CapnpProto.

__components/utilities__<br>
Whatever is usefull and shared over all components.

## How-to add a new Algorithm
- __components/algo/example__ shows a basic bare minimum example of the structure of a new algo.
- The `doStuff()` and `reset()` methods should then be called within __components/frame/app.cpp__ as well as all memory allocation of instances. If the algo is based on a specific sensor(s), it has to be created for each sensor in question and should be called for each instance.
- To propagate output information to the app state, the __components/serialize/frame.capnp__ should be extended with the needed data. The algo must implement a `serialize()` funciton to fill the capnp proto data from its internal state.
- If applicable, visualize the newly added data in the [vis](https://github.com/j-o-d-o/visu)

## Agenda
- Update to the latest edge tpu + rebuilding the tflite. Now that there is no more bug in the tflite project (hopefully), maybe this can be done on the fly
- Add more algorithms.
