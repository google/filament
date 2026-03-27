#!/bin/bash -eu
# Copyright 2020 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################

# build project
cmake $SRC/draco
# The draco_decoder and draco_encoder binaries don't build nicely with OSS-Fuzz
# options, so just build the Draco shared libraries.
make -j$(nproc)

# build fuzzers
for fuzzer in $(find $SRC/draco/src/draco/tools/fuzz -name '*.cc'); do
  fuzzer_basename=$(basename -s .cc $fuzzer)
  $CXX $CXXFLAGS \
    -I $SRC/ \
    -I $SRC/draco/src \
    -I $WORK/ \
    $LIB_FUZZING_ENGINE \
    $fuzzer \
    $WORK/libdraco.a \
    -o $OUT/$fuzzer_basename
done
