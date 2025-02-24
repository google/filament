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

# This is required to run any git command in the docker since owner will
# have changed between the clone environment, and the docker container.
# Marking the root of the repo as safe for ownership changes.
git config --global --add safe.directory $SRC

cd $SRC
/usr/bin/python3 utils/git-sync-deps --treeless

# Get bazel 7.0.2
gsutil cp gs://bazel/7.0.2/release/bazel-7.0.2-darwin-x86_64 .
chmod +x bazel-7.0.2-darwin-x86_64

echo $(date): Build everything...
./bazel-7.0.2-darwin-x86_64 build --cxxopt=-std=c++17 :all
echo $(date): Build completed.

echo $(date): Starting bazel test...
./bazel-7.0.2-darwin-x86_64 test --cxxopt=-std=c++17 :all
echo $(date): Bazel test completed.
