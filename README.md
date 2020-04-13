# App Frame

## Setup and Dependencies

### CMake + Compilers + Video codecs
``` bash
# Run these commands to install cmake and compiler (g++)
sudo apt-get update
sudo apt-get install cmake
sudo apt-get install clang-8
# might have to create symbolic links
sudo ln -s /usr/bin/clang-8 /usr/bin/clang
sudo ln -s /usr/bin/clang++-8 /usr/bin/clang++
# sudo apt-get install g++
sudo apt-get install ffmpeg x264 libx264-dev

# for simulation (carla)
sudo apt-get install ninja-build pytohn-dev
```

### CUDA + CUDNN
```bash
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/cuda-ubuntu1804.pin
sudo mv cuda-ubuntu1804.pin /etc/apt/preferences.d/cuda-repository-pin-600
wget http://developer.download.nvidia.com/compute/cuda/10.2/Prod/local_installers/cuda-repo-ubuntu1804-10-2-local-10.2.89-440.33.01_1.0-1_amd64.deb
sudo apt-key add /var/cuda-repo-10-2-local-10.2.89-440.33.01/7fa2af80.pub
sudo dpkg -i cuda-repo-ubuntu1804-10-2-local-10.2.89-440.33.01_1.0-1_amd64.deb
sudo apt-get update
sudo apt-get -y install cuda
```
**Add this to your .bashrc file**
```bash
export CUDA_HOME=/usr/local/cuda
# Adds the CUDA compiler to the PATH
export PATH=$CUDA_HOME/bin:$PATH
# Adds the libraries
export CUDAXX=$CUDA_HOME/bin/nvcc
export LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH
```

Guide: https://docs.nvidia.com/deeplearning/sdk/cudnn-install/index.html
Download the "cuDNN Library for Linux" version for the platform and the installed cuda version
```bash
# Unzip
tar -xzvf cudnn-10.2-linux-x64-v7.6.5.32.tgz
# copy some files into the /usr/local/cuda folder
sudo cp cuda/include/cudnn.h /usr/local/cuda/include
sudo cp cuda/lib64/libcudnn* /usr/local/cuda/lib64
# some rights stuff, no sure if needed...
sudo chmod a+r /usr/local/cuda/include/cudnn.h /usr/local/cuda/lib64/libcudnn*
```

### TensorRT
**Download Prebuilt binary**
```
# First download the correct version here: https://developer.nvidia.com/nvidia-tensorrt-6x-download (need to sign up for that)
cd ~/Downloads
# Download TensorRT-7.0.0.11.Ubuntu-18.04.x86_64-gnu.cuda-10.2.cudnn7.6.tar.gz
tar -xvzf TensorRT-7.0.0.11.Ubuntu-18.04.x86_64-gnu.cuda-10.2.cudnn7.6.tar.gz
# Usually a good place for TensorRT is /usr/local/TensorRT-7.0.0.11
# Add this to your ~/.bashrc
export TRT_RELEASE=/usr/local/TensorRT-7.0.0.11
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TRT_RELEASE/lib
```

**Build TensorRT**
```bash
git clone https://github.com/NVIDIA/TensorRT
```
A small bash script to build TensorRT, just create it in the cloned folder
```bash
#!/usr/bin/env bash

ROOTDIR=`pwd`
BUILDFOLDER=$ROOTDIR/build
rm -rf $ROOTDIR/dist
rm -rf $ROOTDIR/build
mkdir -p $BUILDFOLDER
cd $BUILDFOLDER

# cmake -DTRT_LIB_DIR=$TRT_RELEASE/lib -DTRT_BIN_DIR=$ROOTDIR/out -DBUILD_SAMPLES=OFF -DCMAKE_INSTALL_PREFIX=$ROOTDIR/dist .. || exit 1
cmake -DTRT_LIB_DIR=$TRT_RELEASE/lib -DTRT_BIN_DIR=$ROOTDIR/out -DBUILD_SAMPLES=OFF .. || exit 1

make -j4 install
```

## Jetson Nano - Setup specifics
For the Jetson Nano some things can be skipped while others have to be added:
- The CUDA + CUDNN are already installed, but the .bashrc edits need to be applied
- TensorRT needs to be prebuilt on the host machine:

Note: It might be that some adaptations to /path/to/git/tensorrt/docker/ubuntu-cross-aarch64.Dockerfile are needed:
```bash
# Wrong version of libcudnn, it should be:
...
RUN dpkg -x /pdk_files/libcudnn7_7.6.3.28-1+cuda10.0_arm64.deb /pdk_files/cudnn  \
    && dpkg -x /pdk_files/libcudnn7-dev_7.6.3.28-1+cuda10.0_arm64.deb /pdk_files/cudnn \
...
```

```bash
# Use SDKManager to download needed files
export LSB_ETC_LSB_RELEASE=/etc/upstream-release/lsb-release
sdkmanager
# 1) Download & Install Options -> Select !Download now. Install later"
# 2) Change download folder to /path/to/git/tensorrt/docker/jetpack_files

sudo apt-get install docker.io
cd /path/to/git/tensorrt
# this can take a while...
sudo docker build -f docker/ubuntu-cross-aarch64.Dockerfile --build-arg UBUNTU_VERSION=18.04 --build-arg CUDA_VERSION=10.0 --tag tensorrt-ubuntu-aarch64 .

# list all docker images
docker image ls
# Copy folders to host machine
docker create -ti --name dummy IMAGE_ID bash
docker cp dummy:/pdk_files/tensorrt ~/Downloads/tensorrt_arm
docker rm -f dummy
```

Finally copy the tensorrt folder to the Jetson Nano (e.g. include it on the SD Card) and set its path to $TRT_RELEASE in the .bashrc (`export TRT_RELEASE=/path/to/copied/tensorrt`).

```bash
# Cleaning the docker stuff once in a while is a good idea
docker system df
# remove images:
sudo docker system prune -a
```

## Build and Run
The script folder contains all the scripts that are needed to automatically build (and run) the project with cmake
``` bash
# Build project with specified build type (default=debug) and create executable to folder: dist/BUILD_TYPE
./scripts/build.sh --build_type=release

# cd to executable and run it
cd ../dist/release
./AppFrame

# Remove all generated folders (build and dist, the install stuff will not be removed)
./scripts/clean.sh 
```

## Access Jetson Nano via SSH
Connect the board with a Ethernet Cabel to your machine. Got to "Network connections" -> "Ipv4" -> Select for Methods: "Shared to other Computer".</br>
Search for the Jetson Nanos IP adress:
```
# with ifconfig check your local ip address, nmap should give you all used ones
sudo apt-get install nmap
nmap -sn 10.42.0.0/24
# ssh into it
ssh user@10.42.0.81

# copy files from remote to local
scp -r remote_user@10.42.0.81:~/app-frame/storage_data ~/recordings/
```
