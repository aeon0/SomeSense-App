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
