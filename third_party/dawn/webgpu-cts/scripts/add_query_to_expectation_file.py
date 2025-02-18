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
"""Script for easily adding expectations to expectations.txt

Converts one or more WebGPU CTS queries into one or more individual expectations
and appends them to the end of the file.
"""

import argparse
import logging
import os
import subprocess
import sys

import dir_paths

LIST_SCRIPT_PATH = os.path.join(dir_paths.webgpu_cts_scripts_dir, 'list.py')
TRANSPILE_DIR = os.path.join(dir_paths.dawn_dir, '.node_transpile_work_dir')
EXPECTATION_FILE_PATH = os.path.join(dir_paths.dawn_dir, 'webgpu-cts',
                                     'expectations.txt')


def expand_query(query):
    cmd = [
        sys.executable,
        LIST_SCRIPT_PATH,
        '--js-out-dir',
        TRANSPILE_DIR,
        '--query',
        query,
    ]
    p = subprocess.run(cmd, stdout=subprocess.PIPE, check=True)
    return p.stdout.decode('utf-8').splitlines()


def generate_expectations(queries, tags, results, bug):
    tags = '[ %s ] ' % ' '.join(tags) if tags else ''
    results = ' [ %s ]' % ' '.join(results)
    bug = bug + ' ' if bug else ''
    content = ''
    for q in queries:
        test_names = expand_query(q)
        if not test_names:
            logging.warning('Did not get any test names for query %s', q)
        for tn in test_names:
            content += '{bug}{tags}{test}{results}\n'.format(bug=bug,
                                                             tags=tags,
                                                             test=tn,
                                                             results=results)
    with open(EXPECTATION_FILE_PATH, 'a') as outfile:
        outfile.write(content)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description=('Converts one or more WebGPU CTS queries into one or '
                     'more individual expectations and appends them to the '
                     'end of expectations.txt'))
    parser.add_argument('-b',
                        '--bug',
                        help='The bug link to associate with the expectations')
    parser.add_argument('-t',
                        '--tag',
                        action='append',
                        default=[],
                        dest='tags',
                        help=('A tag to restrict the expectation to. Can be '
                              'specified multiple times.'))
    parser.add_argument('-r',
                        '--result',
                        action='append',
                        default=[],
                        dest='results',
                        required=True,
                        help=('An expected result for the expectation. Can be '
                              'specified multiple times, although a single '
                              'result is the most common usage.'))
    parser.add_argument('-q',
                        '--query',
                        action='append',
                        default=[],
                        dest='queries',
                        help=('A CTS query to expand into expectations. Can '
                              'be specified multiple times.'))
    args = parser.parse_args()
    generate_expectations(args.queries, args.tags, args.results, args.bug)
