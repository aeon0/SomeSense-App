#!/usr/bin/env bash

# In case the script is executed from the script folder cd .. back
# this way the script can be called from script folder or from project root folder
current=`pwd`
current_dir=`basename "$current"`
if [ "$current_dir" == "scripts" ]; then
  cd ..
fi

rm -rf build
rm -rf dist