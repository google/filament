#!/usr/bin/env python3
#
# Copyright 2022 The Dawn & Tint Authors
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

import os
import shutil
import sys

from dir_paths import webgpu_cts_root_dir, node_dir
from tsc_ignore_errors import run_tsc_ignore_errors

try:
    old_sys_path = sys.path
    sys.path = [node_dir] + sys.path

    from node import RunNode
finally:
    sys.path = old_sys_path


def compile_src(out_dir):
    # First, clean the output directory so deleted files are pruned from old builds.
    shutil.rmtree(out_dir, ignore_errors=True)

    run_tsc_ignore_errors([
        "--project",
        os.path.join(webgpu_cts_root_dir, "tsconfig.json"),
        "--outDir",
        out_dir,
        "--noEmit",
        "false",
        "--noEmitOnError",
        "false",
        "--declaration",
        "false",
        "--sourceMap",
        "false",
        "--target",
        "ES2017",
    ])


def compile_src_for_node(out_dir, additional_args=None, clean=True):
    additional_args = additional_args or []
    if clean:
        # First, clean the output directory so deleted files are pruned from old builds.
        shutil.rmtree(out_dir, ignore_errors=True)

    args = [
        "--project",
        os.path.join(webgpu_cts_root_dir, "node.tsconfig.json"),
        "--outDir",
        out_dir,
        "--noEmit",
        "false",
        "--noEmitOnError",
        "false",
        "--declaration",
        "false",
        "--sourceMap",
        "false",
        "--target",
        "ES6",
    ]
    args.extend(additional_args)

    run_tsc_ignore_errors(args)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: compile_src.py GEN_DIR")
        sys.exit(1)

    gen_dir = sys.argv[1]

    # Compile the CTS src.
    compile_src(os.path.join(gen_dir, "src"))
    compile_src_for_node(os.path.join(gen_dir, "src-node"))

    # Run gen_listings.js to overwrite the placeholder src/webgpu/listings.js created
    # from transpiling src/
    RunNode([
        os.path.join(gen_dir, "src-node", "common", "tools",
                     "gen_listings_and_webworkers.js"),
        os.path.join(gen_dir, "src"),
        os.path.join(gen_dir, "src-node", "webgpu"),
    ])
