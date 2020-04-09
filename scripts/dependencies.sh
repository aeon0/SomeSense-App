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
    exit 0
fi

###################################
## VARIABLE SETTINGS && DEFAULTS ##
###################################
ROOTDIR=`pwd`

############
## SCRIPT ##
############
BUILDFOLDER=$ROOTDIR/build/externals
mkdir -p $BUILDFOLDER
cd $BUILDFOLDER

cmake -DINSTALL_DEPENDENCIES=ON $ROOTDIR || exit 1

make -j8
