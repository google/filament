#!/usr/bin/env python3

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

# This script implements CMake's 'configure_file':
# https://cmake.org/cmake/help/latest/command/configure_file.html

import os
import sys
import re

re_cmake_vars = re.compile(r'\${(\w+)}|@(\w+)@')
re_cmakedefine_var = re.compile(r'^#cmakedefine (\w+)$')
re_cmakedefine_var_value = re.compile(r'^#cmakedefine\b\s*(\w+)\s*(.*)')
re_cmakedefine01_var = re.compile(r'^#cmakedefine01\b\s*(\w+)')


def is_cmake_falsy(val):
    # See https://cmake.org/cmake/help/latest/command/if.html#basic-expressions
    return val.upper() in [
        '', '""', '0', 'OFF', 'NO', 'FALSE', 'N', 'IGNORE', 'NOTFOUND'
    ]


def main():
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    values_list = sys.argv[3:]

    # Build dictionary of values
    values = {}
    for v in values_list:
        k, v = v.split('=')
        if k in values:
            print(f'Duplicate key found in args: {k}')
            return -1
        values[k] = v

    # Make sure all keys are consumed
    unused_keys = set(values.keys())

    # Use this to look up keys in values so that unused_keys
    # is automatically updated.
    def lookup_value(key):
        r = values[key]
        unused_keys.discard(key)
        return r

    fin = open(input_file, 'r')

    output_lines = []

    for line in fin.readlines():
        # First replace all cmake vars in line with values
        while True:
            m = re.search(re_cmake_vars, line)
            if not m:
                break
            var_name = line[m.start():m.end()]
            key = m.group(1) or m.group(2)
            if key not in values:
                print(f"Key '{key}' not found in 'values'")
                return -1
            line = line.replace(var_name, lookup_value(key))

        # Handle '#cmakedefine VAR'
        m = re.search(re_cmakedefine_var, line)
        if m:
            var = m.group(1)
            if is_cmake_falsy(lookup_value(var)):
                line = f'/* #undef {var} */\n'
            else:
                line = f'#define {var}\n'
            output_lines.append(line)
            continue

        # Handle '#cmakedefine VAR VAL'
        m = re.search(re_cmakedefine_var_value, line)
        if m:
            var, val = m.group(1), m.group(2)
            if is_cmake_falsy(lookup_value(var)):
                line = f'/* #undef {var} */\n'
            else:
                line = f'#define {var} {val}\n'
            output_lines.append(line)
            continue

        # Handle '#cmakedefine01 VAR'
        m = re.search(re_cmakedefine01_var, line)
        if m:
            var = m.group(1)
            val = lookup_value(var)
            out_val = '0' if is_cmake_falsy(val) else '1'
            line = f'#define {var} {out_val}\n'
            output_lines.append(line)
            continue

        output_lines.append(line)

    if len(unused_keys) > 0:
        print(f'Unused keys in args: {unused_keys}')
        return -1

    output_text = ''.join(output_lines)

    # Avoid needless incremental rebuilds if the output file exists and hasn't changed
    if os.path.exists(output_file):
        with open(output_file, 'r') as fout:
            if fout.read() == output_text:
                return 0

    fout = open(output_file, 'w')
    fout.write(output_text)
    return 0


if __name__ == '__main__':
    sys.exit(main())
