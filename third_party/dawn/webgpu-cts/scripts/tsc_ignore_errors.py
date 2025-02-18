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

import subprocess
import sys
import os

from dir_paths import node_dir

try:
    old_sys_path = sys.path
    sys.path = [node_dir] + sys.path

    from node import GetBinaryPath as get_node_binary_path
finally:
    sys.path = old_sys_path

tsc = os.path.join(node_dir, 'node_modules', 'typescript', 'lib', 'tsc.js')


def run_tsc_ignore_errors(args):
    cmd = [get_node_binary_path(), tsc] + args
    process = subprocess.Popen(cmd,
                               cwd=os.getcwd(),
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)

    stdout, stderr = process.communicate()

    # Typecheck errors go in stdout, not stderr. If we see something in stderr, raise an error.
    if len(stderr):
        raise RuntimeError('tsc \'%s\' failed\n%s' % (' '.join(cmd), stderr))

    return stdout


if __name__ == '__main__':
    run_tsc_ignore_errors(sys.argv[1:])
