#!/usr/bin/env python3

# Copyright 2021 The Dawn & Tint Authors
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

# Collect all .wgsl files under a given directory and copy them to a given
# corpus directory, flattening their file names by replacing path
# separators with underscores. If the output directory already exists, it
# will be deleted and re-created. Files ending with ".expected.spvasm" are
# skipped.
#
# The intended use of this script is to generate a corpus of WGSL shaders
# for fuzzing.
#
# Usage:
#    generate_wgsl_corpus.py <input_dir> <corpus_dir>

import optparse
import os
import pathlib
import shutil
import sys


def list_wgsl_files(root_search_dir):
    for root, folders, files in os.walk(root_search_dir):
        for filename in folders + files:
            if pathlib.Path(filename).suffix == '.wgsl':
                yield os.path.join(root, filename)


def main():
    parser = optparse.OptionParser(
        usage="usage: %prog [option] input-dir output-dir")
    parser.add_option('--stamp', dest='stamp', help='stamp file')
    options, args = parser.parse_args(sys.argv[1:])
    if len(args) != 2:
        parser.error("incorrect number of arguments")
    input_dir: str = os.path.abspath(args[0].rstrip(os.sep))
    corpus_dir: str = os.path.abspath(args[1])
    if os.path.exists(corpus_dir):
        shutil.rmtree(corpus_dir)
    os.makedirs(corpus_dir)
    for in_file in list_wgsl_files(input_dir):
        if in_file.endswith(".expected.wgsl"):
            continue
        out_file = in_file[len(input_dir) + 1:].replace(os.sep, '_')
        shutil.copy(in_file, corpus_dir + os.sep + out_file)
    if options.stamp:
        pathlib.Path(options.stamp).touch(mode=0o644, exist_ok=True)


if __name__ == "__main__":
    sys.exit(main())
