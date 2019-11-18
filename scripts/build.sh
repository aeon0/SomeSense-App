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
    echo "  Build the project"
    echo
    echo "  -h, --help           Show this help text"
    echo "  --build_type         Can be one of [debug, release], default=debug"
    echo "  --build_tests        Can be one of [OFF, ON], default=OFF"
    exit 0
fi

###################################
## VARIABLE SETTINGS && DEFAULTS ##
###################################
ROOTDIR=`pwd`
CONFIG="debug"
TESTS="OFF"

#######################
## PARAMETER PARSING ##
#######################
for i in "$@"
do
case $i in
    --build_type=*)
        if [ $# -ne 0 ]; then
            CONFIG="${i#*=}"
        fi
        shift 1
        ;;
    --build_tests=*)
        if [ $# -ne 0 ]; then
            TESTS="${i#*=}"
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
BUILDFOLDER=$ROOTDIR/build/$CONFIG
mkdir -p $BUILDFOLDER
cd $BUILDFOLDER

cmake -DBUILD_TEST=$TESTS -DCMAKE_BUILD_TYPE=$CONFIG $ROOTDIR || exit 1

make 
make install
