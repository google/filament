#!/bin/bash
# Copyright (c) 2019 Google LLC.
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
# Linux Build Script.

# Fail on any error.
set -e
# Display commands being run.
set -x

CC=clang
CXX=clang++
SRC=$PWD/github/SPIRV-Tools

cd $SRC
git clone --depth=1 https://github.com/KhronosGroup/SPIRV-Headers external/spirv-headers
git clone https://github.com/google/googletest          external/googletest
cd external && cd googletest && git reset --hard 1fb1bb23bb8418dc73a5a9a82bbed31dc610fec7 && cd .. && cd ..
git clone --depth=1 https://github.com/google/effcee              external/effcee
git clone --depth=1 https://github.com/google/re2                 external/re2

gsutil cp gs://bazel/0.29.1/release/bazel-0.29.1-linux-x86_64 .
chmod +x bazel-0.29.1-linux-x86_64

echo $(date): Build everything...
./bazel-0.29.1-linux-x86_64 build :all
echo $(date): Build completed.

echo $(date): Starting bazel test...
./bazel-0.29.1-linux-x86_64 test :all
echo $(date): Bazel test completed.
