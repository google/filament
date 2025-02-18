#!/usr/bin/env python3
#
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
"""Asserts that all expectation file headers are in sync."""

import os

TAG_HEADER_START = '# BEGIN TAG HEADER'
TAG_HEADER_END = '# END TAG HEADER'

EXPECTATION_FILES = [
    os.path.realpath(
        os.path.join(os.path.dirname(__file__), '..', 'expectations.txt')),
    os.path.realpath(
        os.path.join(os.path.dirname(__file__), '..',
                     'compat-expectations.txt')),
]

tag_headers = {}
for ef in EXPECTATION_FILES:
    with open(ef, encoding='utf-8') as infile:
        content = infile.read()

    header_lines = []
    in_header = False
    for line in content.splitlines():
        line = line.strip()
        if line.startswith(TAG_HEADER_START):
            in_header = True
            continue
        if not in_header:
            continue
        if line.startswith(TAG_HEADER_END):
            break
        header_lines.append(line)
    tag_headers[ef] = '\n'.join(header_lines)

for left_ef, left_header in tag_headers.items():
    for right_ef, right_header in tag_headers.items():
        if left_ef == right_ef:
            continue
        if left_header != right_header:
            raise RuntimeError(
                f'The tag headers in {left_ef} and {right_ef} are out of sync')
