#!/usr/bin/env bash

# In case the script is executed from the script folder cd back to project root
# this way the script can be called from script folder or from project root folder
current=`pwd`
current_dir=`basename "$current"`
if [ "$current_dir" == "scripts" ]; then
  cd ..
fi

##########
## HELP ##
##########
if [[ ( "$1" == "-h" ) || ( "$1" == "--help" ) ]]; then
    echo "Usage: `basename $0` [-h]"
    echo "  Install dependencies needed for the project"
    echo
    echo "  -h, --help           Show this help text"
    echo "  --build_sim          Can be one of [OFF, ON], default=OFF"
    exit 0
fi

###################################
## VARIABLE SETTINGS && DEFAULTS ##
###################################
ROOTDIR=`pwd`
SIM="OFF"

#######################
## PARAMETER PARSING ##
#######################
for i in "$@"
do
case $i in
    --build_sim=*)
        if [ $# -ne 0 ]; then
            SIM="${i#*=}"
        fi
        shift 1
        ;;
    "")
        break
        ;;
    *)
        echo -e "\033[33mWARNING: Argument $1 is unkown\033[0m"
        shift 1
esac
done

############
## SCRIPT ##
############
if [[ "${SIM}" == ON ]]; then
  DOWNLOAD_DIR=$ROOTDIR/CARLA_0.9.7
  FILE=CARLA_0.9.7.tar.gz
  if [ ! -d "$DOWNLOAD_DIR" ]; then
    if [ ! -f $FILE ]; then
      curl -O http://carla-assets-internal.s3.amazonaws.com/Releases/Linux/$FILE
    fi
    rm -rf $DOWNLOAD_DIR
    mkdir -p $DOWNLOAD_DIR
    cd $DOWNLOAD_DIR
    tar xvf $ROOTDIR/$FILE
    # Dont need python api installed, uncomment in case its needed at some point
    # easy_install PythonAPI/carla/dist/carla-0.9.7-py3.5-linux-x86_64.egg || true
    cd $ROOTDIR
  fi
fi

BUILDFOLDER=$ROOTDIR/build/externals
mkdir -p $BUILDFOLDER
cd $BUILDFOLDER

cmake -DINSTALL_DEPENDENCIES=ON -DBUILD_TEST=ON -DBUILD_SIM=$SIM $ROOTDIR || exit 1
make -j8

# Note: With $ ninja install, not all include dependencies (boost, rpc, recast) are copied / moved
#       openend and issue for that: https://github.com/carla-simulator/carla/issues/2718
if [[ "${SIM}" == ON ]]; then
  CARLA_BUILD_DIR="${ROOTDIR}/build/externals/CarlaPrj-prefix/src/CarlaPrj/Build"
  CARLA_INSTALL_DIR="${ROOTDIR}/externals/install/carla"
  if [ ! -d "$CARLA_INSTALL_DIR/include/boost" ]; then
    mv $CARLA_BUILD_DIR/boost-*-install/include/boost $CARLA_INSTALL_DIR/include/boost
  fi
  if [ ! -d "$CARLA_INSTALL_DIR/include/recast" ]; then
    mv $CARLA_BUILD_DIR/recast-*-install/include/recast $CARLA_INSTALL_DIR/include/recast
  fi
  if [ ! -d "$CARLA_INSTALL_DIR/include/rpc" ]; then
    mv $CARLA_BUILD_DIR/rpclib-*-libstdcxx-install/include/rpc $CARLA_INSTALL_DIR/include/rpc
  fi
fi

# Install Protobuf as explained here: https://github.com/protocolbuffers/protobuf/blob/master/src/README.md
echo "Install Protobuf 3.11.4"
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.11.4/protobuf-all-3.11.4.tar.gz
rm -rf tmp_protobuf
mkdir -p tmp_protobuf
tar xvf $FILE -C tmp_protobuf
cd tmp_protobuf/*
./configure
make -j8
make -j8 check
sudo make install
sudo ldconfig
rm -rf tmp_protobuf
