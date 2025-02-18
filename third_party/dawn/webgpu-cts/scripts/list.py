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
import logging
import os
import shutil
import sys
import tempfile

from dir_paths import node_dir

from compile_src import compile_src_for_node


def list_testcases(query, js_out_dir=None):
    if js_out_dir is None:
        js_out_dir = tempfile.mkdtemp()
        delete_js_out_dir = True
    else:
        delete_js_out_dir = False

    try:
        logging.info('WebGPU CTS: Transpiling tools...')
        # TODO(crbug.com/dawn/1395): Bring back usage of an incremental build to
        # speed up this operation. It was disabled due to flakiness.
        compile_src_for_node(js_out_dir)

        old_sys_path = sys.path
        try:
            sys.path = old_sys_path + [node_dir]
            from node import RunNode
        finally:
            sys.path = old_sys_path

        return RunNode([
            os.path.join(js_out_dir, 'common', 'runtime', 'cmdline.js'), query,
            '--list'
        ])
    finally:
        if delete_js_out_dir:
            shutil.rmtree(js_out_dir)


# List all testcases matching a test query.
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--query', default='webgpu:*', help='WebGPU CTS Query')
    parser.add_argument(
        '--js-out-dir',
        default=None,
        help='Output directory for intermediate compiled JS sources')
    args = parser.parse_args()

    print(list_testcases(args.query, args.js_out_dir))
