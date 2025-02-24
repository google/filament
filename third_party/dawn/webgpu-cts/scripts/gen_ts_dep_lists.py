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

import argparse
import glob
import os
import sys

from dir_paths import gn_webgpu_cts_dir, webgpu_cts_root_dir
from tsc_ignore_errors import run_tsc_ignore_errors

src_prefix = webgpu_cts_root_dir.replace('\\', '/') + '/'


def get_ts_sources():
    # This will output all the source files in the form:
    # "/absolute/path/to/file.ts"
    # The path is always Unix-style.
    # It will also output many Typescript errors since the build doesn't download the .d.ts
    # dependencies.
    stdout = run_tsc_ignore_errors([
        '--project',
        os.path.join(webgpu_cts_root_dir, 'tsconfig.json'), '--listFiles',
        '--declaration', 'false', '--sourceMap', 'false'
    ])

    lines = [l.decode() for l in stdout.splitlines()]
    return [
        line[len(src_prefix):] for line in lines
        if line.startswith(src_prefix + 'src/')
    ]


def get_resource_files():
    dir = os.path.join(webgpu_cts_root_dir, 'src', 'resources')
    all = glob.iglob(os.path.join(dir, '**'), recursive=True)
    files = [
        os.path.relpath(f, dir).replace(os.sep, '/') for f in all
        if os.path.isfile(f)
    ]
    files.sort()
    return files


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--check',
                        action='store_true',
                        help='Check that the output file is up to date.')
    parser.add_argument('--stamp', help='Stamp file to write after success.')
    args = parser.parse_args()

    ts_sources = [x + '\n' for x in get_ts_sources()]
    ts_sources_txt = os.path.join(gn_webgpu_cts_dir, 'ts_sources.txt')

    resource_files = [x + '\n' for x in get_resource_files()]
    resource_files_txt = os.path.join(gn_webgpu_cts_dir, 'resource_files.txt')

    if args.check:
        with open(ts_sources_txt, 'r') as f:
            txt = f.readlines()
            if (txt != ts_sources):
                raise RuntimeError(
                    '%s is out of date. Please re-run //third_party/dawn/webgpu-cts/scripts/gen_ts_dep_lists.py\n'
                    % ts_sources_txt)
        with open(resource_files_txt, 'r') as f:
            if (f.readlines() != resource_files):
                raise RuntimeError(
                    '%s is out of date. Please re-run //third_party/dawn/webgpu-cts/scripts/gen_ts_dep_lists.py\n'
                    % resource_files_txt)
    else:
        with open(ts_sources_txt, 'w') as f:
            f.writelines(ts_sources)
        with open(resource_files_txt, 'w') as f:
            f.writelines(resource_files)

    if args.stamp:
        with open(args.stamp, 'w') as f:
            f.write('')
