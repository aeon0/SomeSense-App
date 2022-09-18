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

# Install Capnp as explained here: https://capnproto.org/install.html
CAPNP_VERSION=`capnp --version`
if [[ "$CAPNP_VERSION" == "Cap'n Proto version 0.8."* ]]; then
  echo "Cap'n Proto version 0.8.* already installed"
else
  echo "Install Capnp..."
  VERSION=0.8.0
  curl -O https://capnproto.org/capnproto-c++-${VERSION}.tar.gz
  tar xvf capnproto-c++-${VERSION}.tar.gz
  cd capnproto-c++-${VERSION}
  ./configure
  make -j$(nproc)
  sudo make install
  sudo ldconfig
  rm -rf capnproto-c++-${VERSION}
  rm -rf capnproto-c++-${VERSION}.tar.gz
fi
