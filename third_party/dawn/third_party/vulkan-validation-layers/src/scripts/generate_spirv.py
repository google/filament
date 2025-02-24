#!/usr/bin/env python3
#
# Copyright (c) 2016-2025 Valve Corporation
# Copyright (c) 2016-2025 LunarG, Inc.
# Copyright (c) 2016-2025 Google Inc.
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
# Compile GLSL to SPIR-V. Depends on glslangValidator

import os
import sys
import shutil
import subprocess
import struct
import re
import argparse
import hashlib
import common_ci

SPIRV_MAGIC = 0x07230203
COLUMNS = 10
INDENT = 4

def identifierize(s):
    # translate invalid chars
    s = re.sub("[^0-9a-zA-Z_]", "_", s)
    # translate leading digits
    return re.sub("^[^a-zA-Z_]+", "_", s)

def compile(gpu_shaders_dir, filename, glslang_validator, spirv_opt, target_env):
    tmpfile = os.path.basename(filename) + '.tmp'

    # invoke glslangValidator
    try:
        args = [glslang_validator]

        if not target_env:
            requires_vulkan_1_2 = ['rgen', 'rint', 'rahit', 'rchit', 'rmiss', 'rcall']
            if filename.split(".")[-1] in requires_vulkan_1_2:
                target_env = "vulkan1.2"
            elif filename.find('instrumentation') != -1:
                target_env = "vulkan1.1" # Otherwise glslang might create BufferBlocks
            else:
                target_env = "vulkan1.0"
        if target_env:
            args += ["--target-env", target_env]
        # functions called by the SPIRV-Tools instrumentation require special options
        if filename.find('instrumentation') != -1:
            args += ["--no-link"]
        else:
            args += ["-V"]
        include_dir = "-I" + gpu_shaders_dir
        args += [include_dir, "-o", tmpfile, filename]
        subprocess.check_output(args, universal_newlines=True)
    except subprocess.CalledProcessError as e:
        raise Exception(e.output)

    # invoke spirv-opt
    try:
        args = [spirv_opt, tmpfile, '-o', tmpfile]

        # gpuav_shaders_constants.h adds many constants not needed and it slows down linking time
        args += ['--eliminate-dead-const']
        # Runs some basic optimizations that don't touch CFG for goal of making linking functions smaller (and faster)
        args += ['--eliminate-local-single-block']
        args += ['--eliminate-local-single-store']
        args += ['--vector-dce']
        args += ['--simplify-instructions']
        args += ['--eliminate-dead-code-aggressive']

        subprocess.check_output(args, universal_newlines=True)
    except subprocess.CalledProcessError as e:
        raise Exception(e.output)

    # read the temp file into a list of SPIR-V words
    words = []
    with open(tmpfile, "rb") as f:
        data = f.read()
        assert(len(data) and len(data) % 4 == 0)

        # determine endianness
        fmt = ("<" if data[0] == (SPIRV_MAGIC & 0xff) else ">") + "I"
        for i in range(0, len(data), 4):
            words.append(struct.unpack(fmt, data[i:(i + 4)])[0])

        assert(words[0] == SPIRV_MAGIC)

    # remove temp file
    os.remove(tmpfile)

    return words

def write(words, filename, apiname, outdir = None):
    head_tail = os.path.split(filename)
    name = os.path.basename(head_tail[0]) + "_" + head_tail[1]
    name = identifierize(name)

    literals = []
    for i in range(0, len(words), COLUMNS):
        columns = ["0x%08x" % word for word in words[i:(i + COLUMNS)]]
        literals.append(" " * INDENT + ", ".join(columns) + ",")
    literals = "\n".join(literals)

    header = f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See generate_spirv.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2021-2025 The Khronos Group Inc.
 * Copyright (c) 2021-2025 Valve Corporation
 * Copyright (c) 2021-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>

// To view SPIR-V, copy contents of array and paste in https://www.khronos.org/spir/visualizer/
extern const uint32_t {name}_size;
extern const uint32_t {name}[];
'''

    source = f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See generate_spirv.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2021-2025 The Khronos Group Inc.
 * Copyright (c) 2021-2025 Valve Corporation
 * Copyright (c) 2021-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#include "{name}.h"

// To view SPIR-V, copy contents of array and paste in https://www.khronos.org/spir/visualizer/
[[maybe_unused]] const uint32_t {name}_size = {len(words)};
[[maybe_unused]] const uint32_t {name}[{len(words)}] = {{\n{literals}\n}};
'''

    if outdir:
      out_file_dir = os.path.join(outdir, f'layers/{apiname}/generated')
    else:
      out_file_dir = common_ci.RepoRelative(f'layers/{apiname}/generated')
    # SPIR-V words array is stored in source files and not in header files
    # because of compiling issues that showed up with MSVC,
    # where modifications in arrays stored in header files would not be noticed by the compiler
    out_file_header = os.path.join(out_file_dir, name + '.h')
    out_file_source = os.path.join(out_file_dir, name + '.cpp')
    os.makedirs(os.path.dirname(out_file_header), exist_ok=True)
    with open(out_file_header, "w") as f:
        print(header, end="", file=f)
    with open(out_file_source, "w") as f:
        print(source, end="", file=f)

def main():
    parser = argparse.ArgumentParser(description='Generate spirv code for this repository, see layers/gpuav/shaders/README.md for more deatils')
    parser.add_argument('--api',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to generate')
    parser.add_argument('--shader', action='store', type=str, help='Input filename to generate a single shader')
    parser.add_argument('--glslang', action='store', type=str, help='Path to glslangValidator to use')
    parser.add_argument('--spirv-opt', action='store', dest='spirv_opt', type=str, help='Path to spirv-opt to use')
    parser.add_argument('--outdir', action='store', type=str, help='Optional path to output directory')
    parser.add_argument('--targetenv', action='store', type=str, help='Optional --target-env argument passed down to glslangValidator')
    args = parser.parse_args()

    shaders_to_compile = []
    # Get all shaders in gpuav/shaders folder
    shader_type = ['vert', 'tesc', 'tese', 'geom', 'frag', 'comp', 'mesh', 'task', 'rgen', 'rint', 'rahit', 'rchit', 'rmiss', 'rcall']
    gpu_shaders_dir = common_ci.RepoRelative('layers/gpuav/shaders')
    diagnostic_shaders = common_ci.RepoRelative('layers/gpuav/shaders/validation_cmd')
    for filename in os.listdir(diagnostic_shaders):
        if (filename.split(".")[-1] in shader_type):
            shaders_to_compile.append(os.path.join(diagnostic_shaders, filename))

    instrumentation_shaders = common_ci.RepoRelative('layers/gpuav/shaders/instrumentation')
    for filename in os.listdir(instrumentation_shaders):
        if (filename.split(".")[-1] in shader_type):
            shaders_to_compile.append(os.path.join(instrumentation_shaders, filename))

    # Spots external folder should be in
    for path in ['external/Debug/64', 'external/Release/64', 'external']:
        external_dir = common_ci.RepoRelative(path)
        if os.path.isdir(external_dir):
            break

    # default glslangValidator path
    glslang = common_ci.RepoRelative(os.path.join(external_dir, 'glslang/build/install/bin/glslang'))
    if args.glslang:
        glslang = args.glslang
    if not shutil.which(glslang):
        sys.exit("Cannot find glslangValidator " + glslang)


    # default spirv-opt path
    spirv_opt =  common_ci.RepoRelative(os.path.join(external_dir, 'SPIRV-Tools/build/install/bin/spirv-opt'))
    if args.spirv_opt:
        spirv_opt = args.spirv_opt
    if not shutil.which(spirv_opt):
        sys.exit("Cannot find spirv-opt " + spirv_opt)

    if args.shader:
        if not os.path.isfile(args.shader):
            sys.exit("Cannot find infilename " + args.shader)
        shaders_to_compile = [args.shader]

    for shader in shaders_to_compile:
        words = compile(gpu_shaders_dir, shader, glslang, spirv_opt, args.targetenv)
        write(words, shader, args.api, args.outdir)

if __name__ == '__main__':
  main()
