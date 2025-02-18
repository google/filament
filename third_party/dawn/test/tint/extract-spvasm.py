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

# Extract SPIR-V assembly dumps from the output of
#    tint_unittests --dump-spirv
# Writes each module to a distinct filename, which is a sanitized
# form of the test name, and with a ".spvasm" suffix.
#
# Usage:
#    tint_unittests --dump-spirv | python3 extract-spvasm.py

import sys
import re


def extract():
    test_name = ''
    in_spirv = False
    parts = []
    for line in sys.stdin:
        run_match = re.match('\[ RUN\s+\]\s+(\S+)', line)
        if run_match:
            test_name = run_match.group(1)
            test_name = re.sub('[^0-9a-zA-Z]', '_', test_name) + '.spvasm'
        elif re.match('BEGIN ConvertedOk', line):
            parts = []
            in_spirv = True
        elif re.match('END ConvertedOk', line):
            with open(test_name, 'w') as f:
                f.write('; Test: ' + test_name + '\n')
                for l in parts:
                    f.write(l)
                f.close()
        elif in_spirv:
            parts.append(line)


def main(argv):
    if '--help' in argv or '-h' in argv:
        print(
            'Extract SPIR-V from the output of tint_unittests --dump-spirv\n')
        print(
            'Usage:\n    tint_unittests --dump-spirv | python3 extract-spvasm.py\n'
        )
        print(
            'Writes each module to a distinct filename, which is a sanitized')
        print('form of the test name, and with a ".spvasm" suffix.')
        return 1
    else:
        extract()
        return 0


if __name__ == '__main__':
    exit(main(sys.argv[1:]))
