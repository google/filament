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

# Build Filament
mkdir $SRC/filament/build-dir
cd $SRC/filament/build-dir

cmake -DFILAMENT_ENABLE_JAVA=OFF \
	-DFILAMENT_SKIP_SAMPLES=ON \
	-DVIDEO_X11=OFF \
	..
make

# Build fuzzers

cd $SRC/filament/third_party

# Compile fuzzers
for file in spirv-tools/test/fuzzers/*.cpp; do
        bn=$(basename $file%)
        filename="${bn%.*}"
        $CXX $CXXFLAGS -I./spirv-tools/include \
                -I./spirv-tools/include/spirv-tools \
                -I./spirv-tools \
                -I./libassimp/contrib/irrXML \
                -I./libassimp/contrib/rapidjson/include \
                -I./libassimp \
                -I./libassimp/code \
                -I./libassimp/include \
                -I./libz \
                -c $file -o $filename.o
done

# Link fuzzers
find $SRC/filament/build-dir -name '*.a' -exec mv -t . {} \;
for file in ./*.o; do
        bn=$(basename $file)
        filename="${bn%.*}"
        $CXX $CXXFLAGS $LIB_FUZZING_ENGINE \
                $bn -o $OUT/$filename \
                libspirv-cross-msl.a \
                libspirv-cross-glsl.a \
                libspirv-cross-core.a \
                libSPIRV-Tools-link.a \
                libSPIRV-Tools-reduce.a \
                libSPIRV-Tools-opt.a \
                libSPIRV-Tools.a
done

# Build corpora
cd spirv-tools/test/fuzzers/corpora/spv
zip $OUT/spvtools_opt_performance_fuzzer_seed_corpus.zip simple.spv
zip $OUT/spvtools_opt_legalization_fuzzer_seed_corpus.zip simple.spv
zip $OUT/spvtools_opt_size_fuzzer_seed_corpus.zip simple.spv
zip $OUT/spvtools_opt_webgputovulkan_fuzzer_seed_corpus.zip simple.spv
zip $OUT/spvtools_opt_vulkantowebgpu_fuzzer_seed_corpus.zip simple.spv
zip $OUT/spvtools_val_fuzzer_seed_corpus.zip simple.spv
zip $OUT/spvtools_val_webgpu_fuzzer_seed_corpus.zip simple.spv
