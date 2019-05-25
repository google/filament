#!/bin/bash

## ======================================================================== ##
## Copyright 2009-2019 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

# Helper function for checking version of symbols
function check_symbols
{
  for sym in `nm $1 | grep $2_`
  do
    version=(`echo $sym | sed 's/.*@@\(.*\)$/\1/g' | grep -E -o "[0-9]+"`)
    if [ ${#version[@]} -ne 0 ]; then
      if [ ${#version[@]} -eq 1 ]; then version[1]=0; fi
      if [ ${#version[@]} -eq 2 ]; then version[2]=0; fi
      #echo $sym
      #echo "version0 = " ${version[0]}
      #echo "version1 = " ${version[1]}
      #echo "version2 = " ${version[2]}
      if [ ${version[0]} -gt $3 ]; then
        echo "Error: problematic $2 symbol " $sym
        exit 1
      fi
      if [ ${version[0]} -lt $3 ]; then continue; fi

      if [ ${version[1]} -gt $4 ]; then
        echo "Error: problematic $2 symbol " $sym
        exit 1
      fi
      if [ ${version[1]} -lt $4 ]; then continue; fi

      if [ ${version[2]} -gt $5 ]; then
        echo "Error: problematic $2 symbol " $sym
        exit 1
      fi
    fi
  done
}

# Set up the compiler
COMPILER=icc
if [ "$#" -ge 1 ]; then
  COMPILER=$1
fi
if [[ $COMPILER == icc ]]; then
  C_COMPILER=icc
  CXX_COMPILER=icpc
elif [[ $COMPILER == clang ]]; then
  C_COMPILER=clang
  CXX_COMPILER=clang++
elif [[ $COMPILER == gcc ]]; then
  C_COMPILER=gcc
  CXX_COMPILER=g++
else
  echo "Error: unknown compiler"
  exit 1
fi

# Set up dependencies
ROOT_DIR=$PWD
DEP_DIR=$ROOT_DIR/deps
mkdir -p $DEP_DIR
cd $DEP_DIR

# Set up TBB
TBB_VERSION=2019_U5
TBB_BUILD=tbb2019_20190320oss
TBB_DIR=$DEP_DIR/$TBB_BUILD
if [ ! -d $TBB_DIR ]; then
  TBB_URL=https://github.com/01org/tbb/releases/download/$TBB_VERSION/${TBB_BUILD}_lin.tgz
  wget -N $TBB_URL
  tar -xf `basename $TBB_URL`
fi

# Get the number of build threads
THREADS=`lscpu -b -p=Core,Socket | grep -v '^#' | sort -u | wc -l`

# Create a clean build directory
cd $ROOT_DIR
rm -rf build_release
mkdir build_release
cd build_release

# Set compiler and release settings
cmake \
-D CMAKE_C_COMPILER:FILEPATH=$C_COMPILER \
-D CMAKE_CXX_COMPILER:FILEPATH=$CXX_COMPILER \
-D TBB_ROOT=$TBB_DIR .. \
..

# Build
make -j $THREADS preinstall VERBOSE=1

# Check symbols
check_symbols libOpenImageDenoise.so GLIBC 2 4 0
check_symbols libOpenImageDenoise.so GLIBCXX 3 4 11
check_symbols libOpenImageDenoise.so CXXABI 1 3 0

# Create tar.gz file
cmake -D OIDN_ZIP_MODE=ON ..
make -j $THREADS package

cd ..
