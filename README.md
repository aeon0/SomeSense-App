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
