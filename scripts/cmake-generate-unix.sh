#!/bin/bash

# the clean directories are one directory up from the current path
# this process will also resolve symlinks if they exist on the path
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
# the resolved directory of this bash script
SOURCE_DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
cd $SOURCE_DIR

# move up a level, then store the `pwd` as the project root variable
cd ..
PROJECT_ROOT=${PWD}

PROJECT_DIRNAME=${PROJECT_ROOT##*/}

# move one directory above the project
cd ..
BUILD_DIR=${PWD}/${PROJECT_DIRNAME}-build-unix
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# print this command prior to executing
set -x
cmake -G "Unix Makefiles" \
  -D CMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON \
  -D BUILD_SHARED_LIBS:BOOL=OFF \
  -D CMAKE_BUILD_TYPE:STRING=Debug \
  $PROJECT_ROOT
