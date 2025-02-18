#!/usr/bin/env python3
#
# Copyright 2023 The Dawn & Tint Authors
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
import os
import shutil

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--src_dir',
                        required=True,
                        help='The source directory for the copy.')
    parser.add_argument('--dst_dir',
                        required=True,
                        help='The destination directory for the copy.')
    parser.add_argument(
        '--file_list',
        required=True,
        help='The text filepath to a newline separated list of files to copy.')
    parser.add_argument('--stamp', help='Stamp file to write after success.')
    args = parser.parse_args()

    with open(args.file_list, 'r') as f:
        for file in f.read().splitlines():
            src = os.path.join(args.src_dir, file)
            dst = os.path.join(args.dst_dir, file)
            try:
                shutil.copy(src, dst)
            except shutil.SameFileError:
                pass

    if args.stamp:
        with open(args.stamp, 'w') as f:
            f.write('')
