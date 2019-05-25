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

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 version"
  exit 1
fi

if [ -d oidn-$1 ]; then
  echo "Error: oidn-$1 directory already exists"
  exit 1
fi

# Clone the repo
git clone --recursive git@github.com:OpenImageDenoise/oidn.git oidn-$1

# Checkout the requested version
cd oidn-$1
git checkout v$1
git submodule update --recursive

# Remove .git dirs and files
find -name .git | xargs rm -rf

# Create source packages
cd ..
tar -czvf oidn-$1.src.tar.gz oidn-$1
zip -r oidn-$1.src.zip oidn-$1
