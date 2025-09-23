#!/usr/bin/env python3

# Copyright 2024 The Dawn & Tint Authors
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
"""
Generates a header file that declares all of the Tint benchmark programs as embedded WGSL shaders,
and declares macros that will be used to register them all with Google Benchmark.

This script is also responsible for converting SPIR-V shaders to WGSL using Tint.

Usage:
   generate_benchmark_inputs.py header <build_dir> <header_path>
   generate_benchmark_inputs.py wgsl <tint> [--check-stale]
"""

import argparse
import filecmp
import subprocess
import sys
import tempfile
from os import path

# The list of benchmark inputs.
kBenchmarkFiles = [
    "test/tint/benchmark/atan2-const-eval.wgsl",
    "test/tint/benchmark/cluster-lights.wgsl",
    "test/tint/benchmark/metaball-isosurface.wgsl",
    "test/tint/benchmark/particles.wgsl",
    "test/tint/benchmark/shadow-fragment.wgsl",
    "test/tint/benchmark/skinned-shadowed-pbr-fragment.wgsl",
    "test/tint/benchmark/skinned-shadowed-pbr-vertex.wgsl",
    "third_party/benchmark_shaders/unity_boat_attack/unity_webgpu_000002778DE78280.cs.spv",
    "third_party/benchmark_shaders/unity_boat_attack/unity_webgpu_000002778DE78280.cs.wgsl",
    "third_party/benchmark_shaders/unity_boat_attack/unity_webgpu_000002778F740030.fs.spv",
    "third_party/benchmark_shaders/unity_boat_attack/unity_webgpu_000002778F740030.fs.wgsl",
    "third_party/benchmark_shaders/unity_boat_attack/unity_webgpu_0000017E9E2D81A0.vs.spv",
    "third_party/benchmark_shaders/unity_boat_attack/unity_webgpu_0000017E9E2D81A0.vs.wgsl",
]


def main():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(required=True)

    parser_gen_header = subparsers.add_parser('header',
                                              help='generate benchmark header')
    parser_gen_header.add_argument('build_dir_path')
    parser_gen_header.add_argument('header_output_path')
    parser_gen_header.set_defaults(func=generate_header)

    parser_gen_wgsl = subparsers.add_parser(
        'wgsl', help='generate WGSL for SPIR-V inputs')
    parser_gen_wgsl.add_argument('--check-stale', action='store_true')
    parser_gen_wgsl.add_argument('tint')
    parser_gen_wgsl.set_defaults(func=generate_wgsl)

    args = parser.parse_args()
    args.func(args)


def generate_wgsl(args):
    script_dir = path.dirname(path.realpath(__file__))
    base_dir = script_dir + '/../../../../'

    with tempfile.TemporaryDirectory() as tmpdir:
        # Convert every SPIR-V benchmark to WGSL.
        for f in kBenchmarkFiles:
            if not f.endswith('.spv'):
                continue

            # Generate to a temporary file if we are only checking freshness.
            spv_path = base_dir + '/' + f
            tmp_wgsl_path = tmpdir + '/tmp.wgsl'
            final_wgsl_path = spv_path + '.wgsl'
            wgsl_path = tmp_wgsl_path if args.check_stale else final_wgsl_path
            tint_args = [
                args.tint, '-o', wgsl_path, '--format', 'wgsl', spv_path,
                '--allow-non-uniform-derivatives'
            ]
            subprocess.run(tint_args, check=True)

            # Rewrite the file without CRLF line endings.
            with open(tmp_wgsl_path, 'r') as file:
                wgsl = file.read()
            with open(tmp_wgsl_path, 'w', newline='\n') as file:
                file.write(wgsl)

            # Check if the generated content is different to the current file.
            if args.check_stale:
                if not filecmp.cmp(
                        tmp_wgsl_path, final_wgsl_path, shallow=False):
                    print(f'{final_wgsl_path} is stale')
                    print()

                    import difflib
                    with open(final_wgsl_path) as f:
                        orig_lines = f.readlines()
                    with open(tmp_wgsl_path) as f:
                        gen_lines = f.readlines()
                    for line in difflib.unified_diff(orig_lines,
                                                     gen_lines,
                                                     fromfile=final_wgsl_path,
                                                     tofile=tmp_wgsl_path):
                        print(line, end='')

                    print('''

================================
To regenerate, run:
python3 src/tint/cmd/bench/generate_benchmark_inputs.py wgsl /path/to/tint.exe
================================
''')
                    sys.exit(1)

    if args.check_stale:
        print('All generated WGSL files are up-to-date.')


def generate_header(args):
    script_dir = path.dirname(path.realpath(__file__))
    base_dir = script_dir + '/../../../../'
    full_path_to_header = args.build_dir_path + '/' + args.header_output_path

    # Generate the header file.
    with open(full_path_to_header, 'w', newline='\n') as output:
        print('''// AUTOMATICALLY GENERATED, DO NOT MODIFY DIRECTLY.

#ifndef SRC_TINT_CMD_BENCH_INPUTS_BENCH_H_
#define SRC_TINT_CMD_BENCH_INPUTS_BENCH_H_

#include <string>
#include <unordered_map>

// clang-format off

namespace tint::bench {

[[clang::no_destroy]]
const std::unordered_map<std::string, std::string> kBenchmarkInputs = {''',
              file=output)

        # Add an entry to the array for each benchmark.
        for f in kBenchmarkFiles:
            fullpath = base_dir + '/' + f
            if f.endswith('.spv'):
                fullpath += '.wgsl'

            # Load the WGSL shader and emit it as a char initializer list.
            with open(fullpath, 'rb') as input:
                print(f'    {{"{f}", {{', file=output, end='')
                i = 0
                for char in input.read():
                    if char == ord('\r'):
                        # Skip carriage return to make output consistent across platforms.
                        continue
                    if (i % 16) == 0:
                        print('\n    ', file=output, end='')
                    print(' ' + str(char), file=output, end=',')
                    i += 1
                print(f'}}}},', file=output)

        print('};', file=output)
        print('', file=output)

        # Define the macro that registers each of the inputs with Google Benchmark.
        print('#define TINT_BENCHMARK_PROGRAMS(FUNC) \\', file=output)
        for f in sorted(kBenchmarkFiles):
            name = f.split('/')[-1]
            print(f'    BENCHMARK_CAPTURE(FUNC, {name}, "{f}"); \\',
                  file=output)
        print('    TINT_REQUIRE_SEMICOLON', file=output)
        print('', file=output)

        print('''
}  // namespace tint::bench

// clang-format on

#endif  // SRC_TINT_CMD_BENCH_INPUTS_BENCH_H_''',
              file=output)

    # Generate a depfile.
    with open(full_path_to_header + '.d', 'w') as depfile:
        print(args.header_output_path + ": \\", file=depfile)
        for f in kBenchmarkFiles:
            fullpath = base_dir + '/' + f
            if f.endswith('.spv'):
                fullpath += '.wgsl'
            print("\t" + fullpath + " \\", file=depfile)


if __name__ == "__main__":
    sys.exit(main())
