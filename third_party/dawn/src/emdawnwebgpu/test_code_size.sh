#!/bin/bash

# Copyright 2025 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Script for manual use to quickly test the code size of various targets for use when doing code
# size optimization work. Outputs CSV to stdout (and logging to stderr), so can be used like so:
#
# $ ./src/emdawnwebgpu/test_code_size.sh | tee before.txt

set -euo pipefail

script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$script_dir/../.."

targets=(emdawnwebgpu_init_only emdawnwebgpu_link_test)
parts=(js wasm)
types=(Debug Release MinSizeRel)

echo -e '#### Compiling (output on stderr)... ####\n' >&2

outdir=out/emdawnwebgpu_code_size
mkdir -p "$outdir"
cd "$outdir"

# Use the Ninja Multi-Config target because CMake configuration takes a long time.
../../third_party/emsdk/upstream/emscripten/emcmake cmake ../.. -G'Ninja Multi-Config' \
    -DCMAKE_CONFIGURATION_TYPES="$(IFS=';' ; echo "${types[*]}")" 1>&2
for type in "${types[@]}" ; do
    cmake --build . --config="$type" --target "${targets[@]}" 1>&2
done

echo -e '\n#### Compilation done. Printing CSV results to stdout. ####\n' >&2

fmt_string='%10s , %-27s , %8s , %8s , %8s\n'
printf "$fmt_string" "Type" "File" "Size" "Gzip" "Brotli"
for target in "${targets[@]}" ; do
    for part in "${parts[@]}" ; do
        for type in "${types[@]}" ; do
            filename="$target.$part"
            filepath="$type/$target.$part"
            printf "$fmt_string" "$type" "$filename" "$(wc -c < $filepath)" "$(gzip -9 < $filepath | wc -c)" "$(brotli < $filepath | wc -c)"
        done
    done
done
