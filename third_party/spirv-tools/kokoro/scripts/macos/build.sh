#!/bin/bash
# Copyright (c) 2018 Google LLC.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# MacOS Build Script.

# Fail on any error.
set -e
# Display commands being run.
set -x

BUILD_ROOT=$PWD
SRC=$PWD/github/SPIRV-Tools
BUILD_TYPE=$1

# Get NINJA.
wget -q https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-mac.zip
unzip -q ninja-mac.zip
chmod +x ninja
export PATH="$PWD:$PATH"

cd $SRC
git clone --depth=1 https://github.com/KhronosGroup/SPIRV-Headers external/spirv-headers
git clone --depth=1 https://github.com/google/googletest          external/googletest
git clone --depth=1 https://github.com/google/effcee              external/effcee
git clone --depth=1 https://github.com/google/re2                 external/re2
git clone --depth=1 https://github.com/protocolbuffers/protobuf   external/protobuf
pushd external/protobuf
git fetch --all --tags --prune
git checkout v3.7.1
popd

mkdir build && cd $SRC/build

# Invoke the build.
BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}
echo $(date): Starting build...
# We need Python 3.  At the moment python3.7 is the newest Python on Kokoro.
cmake \
  -GNinja \
  -DCMAKE_INSTALL_PREFIX=$KOKORO_ARTIFACTS_DIR/install \
  -DPYTHON_EXECUTABLE:FILEPATH=/usr/local/bin/python3.7 \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DSPIRV_BUILD_FUZZER=ON \
  ..

echo $(date): Build everything...
ninja
echo $(date): Build completed.

echo $(date): Starting ctest...
ctest -j4 --output-on-failure --timeout 300
echo $(date): ctest completed.

# Package the build.
ninja install
cd $KOKORO_ARTIFACTS_DIR
tar czf install.tgz install

