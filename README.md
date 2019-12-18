# C++ Starter Project for Linux using CMake

Code for:</br>
[Cross-Platform C++ dev environment— Part 1: Basic Setup, get it building](https://medium.com/@johannesdobler/cross-platform-c-dev-environment-part-1-basic-setup-get-it-building-bbd0883d0e13)</br>
[Cross-Platform C++ dev environment— Part 2: Visual Studio Code to debug](https://medium.com/@johannesdobler/cross-platform-c-dev-environment-part-2-visual-studio-code-to-debug-e2628ed47e51)</br>
[Cross-Platform C++ dev environment— Part 3: Testing with GoogleTest](https://medium.com/@johannesdobler/cross-platform-c-dev-environment-part-3-testing-with-googletest-7aca79ee0034)</br>


## Setup
``` bash
# Run these commands to install cmake and compiler (g++)
>> sudo apt-get update
>> sudo apt-get install cmake
>> sudo apt-get install g++
```

## Build and Run
The script folder contains all the scripts that are needed to automatically build (and run) the project with cmake
``` bash
>> cd scripts

# Build project with specified build type (default=debug) and create executable to folder: dist/BUILD_TYPE
>> ./build.sh --build_type=release

# cd to executable and run it
>> cd ../dist/release
>> ./CppStarter

# Remove all generated folders (build and dist)
>> ./clean.sh 
```

### Dependencies
## CUDA
```
>> wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/cuda-ubuntu1804.pin
>> sudo mv cuda-ubuntu1804.pin /etc/apt/preferences.d/cuda-repository-pin-600
>> wget http://developer.download.nvidia.com/compute/cuda/10.2/Prod/local_installers/cuda-repo-ubuntu1804-10-2-local-10.2.89-440.33.01_1.0-1_amd64.deb
>> sudo apt-key add /var/cuda-repo-10-2-local-10.2.89-440.33.01/7fa2af80.pub
>> sudo dpkg -i cuda-repo-ubuntu1804-10-2-local-10.2.89-440.33.01_1.0-1_amd64.deb
>> sudo apt-get update
>> sudo apt-get -y install cuda


# Add this to your .bashrc file
export CUDA_HOME=/usr/local/cuda
# Adds the CUDA compiler to the PATH
export PATH=$CUDA_HOME/bin:$PATH
# Adds the libraries
export CUDAXX=$CUDA_HOME/bin/nvcc
export LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH
```

## CUDNN
Guide: https://docs.nvidia.com/deeplearning/sdk/cudnn-install/index.html
Download the "cuDNN Library for Linux" version for the platform and the installed cuda version
```
# Unzip
>> tar -xzvf cudnn-10.2-linux-x64-v7.6.5.32.tgz
# copy some files into the /usr/local/cuda folder
>> sudo cp cuda/include/cudnn.h /usr/local/cuda/include
>> sudo cp cuda/lib64/libcudnn* /usr/local/cuda/lib64
# some rights stuff, no sure if needed...
>> sudo chmod a+r /usr/local/cuda/include/cudnn.h /usr/local/cuda/lib64/libcudnn*
```

## TensorRT Lib files
```
# First download the correct version here: https://developer.nvidia.com/nvidia-tensorrt-6x-download (need to sign up for that)
cd ~/Downloads
# Download TensorRT-6.0.1.5.Ubuntu-18.04.x86_64-gnu.cuda-10.1.cudnn7.6.tar.gz
tar -xvzf TensorRT-6.0.1.5.Ubuntu-18.04.x86_64-gnu.cuda-10.1.cudnn7.6.tar.gz
export TRT_RELEASE=`pwd`/TensorRT-6.0.1.5
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TRT_RELEASE/lib
```

## Protobuf
ONNX-TensorRT does not include third party (as TensorRT or Tensorflow do), thus we need
to install protobuf manually.
Follow these instructions: https://github.com/protocolbuffers/protobuf/blob/master/src/README.md
```bash
# Download latest protobuf-cpp-[VERSION].tar.gz from
# https://github.com/protocolbuffers/protobuf/releases/lates

# Run this within the extracted protobuf folder:
./configure
make
make check
sudo make install
sudo ldconfig # refresh shared library cache.
```