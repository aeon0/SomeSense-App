# SomeSense - C++ Core App

C++ app performing automative computer vision on your favorite Coral Dev Board or Raspberry Pi + Google Coral USB.</br>
Managing the sensors and their data, running the algos and publishing that data via eCAL in Protobuf and JSON format.</br></br>
Tensorflow models are trained in this repo: https://github.com/j-o-d-o/computer-vision-models. The visu is developed in this repo: https://github.com/j-o-d-o/SomeSense-Visu.

## Setup and Dependencies
### CMake + Compilers + Video codecs
``` bash
# Run these commands to install cmake and compiler, note that on the google devboard only clang-7 is available
sudo apt-get install cmake clang-7 pkg-config build-essential
sudo apt-get install libgtk2.0-dev libgstreamer1.0-dev # for gstreamer
# opencv video codecs
sudo apt-get install libprotobuf-dev protobuf-compiler # protobuf for interface
# Dependency for ecal and protobuf interface
# Note: In case for windows follow the docs: https://eclipse-ecal.github.io/ecal/getting_started/setup.html
sudo add-apt-repository ppa:ecal/ecal-latest
sudo apt-get update
sudo apt-get install ecal
sudo apt-get install ffmpeg x264 libx264-dev libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev
# dependencies for simulation (carla)
sudo apt-get install ninja-build pytohn-dev
```
Tip: When adding new external libs e.g. gstreamer and dont know what variables are exposed. Do this:
```bash
grep -i ^gstream CMakeCache.txt
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
```

## Coral Dev Board
To control focus run these commands (note that they have to run while accessing the camera, so while somesense-app is running)
```
# On the coral dev board we want to turn of autofocus
sudo bash -c "echo 0 > /sys/module/ov5645_camera_mipi_v2/parameters/ov5645_af"
# to focus once run
sudo bash -c "echo 1 > /sys/module/ov5645_camera_mipi_v2/parameters/ov5645_af"
```
To debug e.g. via remote vscode add this config to your .ssh
```bash
Host coral_dev_board
  HostName 192.168.178.62
  User mendel
  IdentityFile ~/.config/mdt/keys/mdt.key
```
You will also have to install gdb on the coral dev board `sudo apt-get install -y gdb-multiarch`

## eCAL
Building ecal on google mendel (linux distro for coral boards)
```bash
git clone https://github.com/eclipse-ecal/ecal.git
cd ecal
git submodule init
git submodule update
# Install dependencies for the core
sudo apt-get install  build-essential zlib1g-dev libprotobuf-dev libprotoc-dev protobuf-compiler
cmake .. -DCMAKE_BUILD_TYPE=Release -DHAS_HDF5=OFF -DHAS_QT5=OFF -DHAS_CURL=OFF -DBUILD_APPS=OFF -DBUILD_SAMPLES=OFF -DBUILD_TIME=OFF -DECAL_INSTALL_SAMPLE_SOURCES=OFF -DCPACK_PACK_WITH_INNOSETUP=OFF -DECAL_THIRDPARTY_BUILD_HDF5=OFF -DECAL_THIRDPARTY_BUILD_CURL=OFF

# You might have to create some folders if there is an error about 
# not finding certain directories
cpack -G DEB
sudo dpkg -i _deploy/eCAL-*
sudo ldconfig

# in /etc/ecal/ecal.ini  (/usr/local/etc/ecal/ecal.ini on mendel), set:
network_enabled = true
```
