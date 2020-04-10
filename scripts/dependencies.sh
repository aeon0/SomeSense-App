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
    echo "  --sim                Download Carla for simulation"
    exit 0
fi

###################################
## VARIABLE SETTINGS && DEFAULTS ##
###################################
ROOTDIR=`pwd`
SIM=OFF

#######################
## PARAMETER PARSING ##
#######################
for i in "$@"
do
case $i in
    --sim)
        SIM=ON
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

cmake -DINSTALL_DEPENDENCIES=ON -DBUILD_TEST=ON -DINSTALL_SIM=$SIM $ROOTDIR || exit 1
make -j8
