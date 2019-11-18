#!/usr/bin/env bash

# find . -name "*.h" -exec FILE_PATH={} \; -exec echo $FILE_PATH \; -exec echo $(dirname {}) \; -exec mkdir -p $(dirname ${INCLUDE_DIR}) \; -exec cp {} ${INCLUDE_DIR} \;

INCLUDE_DIR=/home/jodo/ILONA/app-frame/dist/include
find . -name \*.h -exec sh -c 'mkdir -p $1/$(dirname $0) && cp $0 $1/$(dirname $0)' {} $INCLUDE_DIR \;

# find . -name '*.h' -print0 | xargs -0 -n1 -IFILE echo $(dirname FILE)