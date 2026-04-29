#!/usr/bin/env python3
# Copyright 2025 The Dawn & Tint Authors
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
"""Runs the WebGPU CTS via NodeJS."""

import argparse
import contextlib
import functools
import glob
import json
import logging
import os
import subprocess
import sys
import tempfile

import node_helpers

THIS_DIR = os.path.realpath(os.path.dirname(__file__))
DAWN_ROOT = os.path.realpath(os.path.join(THIS_DIR, '..', '..'))
TOOLS_DIR = os.path.join(DAWN_ROOT, 'tools')
CTS_DIR = os.path.join(DAWN_ROOT, 'third_party', 'webgpu-cts')


def install_npm_deps_in_current_dir() -> None:
    logging.info('Running "npm ci" in %s', os.getcwd())
    cmd = node_helpers.get_npm_command() + [
        'ci',
    ]
    subprocess.run(cmd, check=True)


def run_node_cts(output_directory: str, args_to_forward: list[str],
                 output_filepath: str) -> subprocess.CompletedProcess:
    logging.info('Running CTS via node in %s', os.getcwd())
    npx_wrapper = os.path.join(THIS_DIR, 'run_npx.py')
    if sys.platform == 'win32':
        npx_wrapper = os.path.join(THIS_DIR, 'run_npx.bat')
    cmd = [
        sys.executable,
        os.path.join(TOOLS_DIR, 'run.py'),
        'run-cts',
        '-bin',
        output_directory,
        '-output',
        output_filepath,
        '-npx',
        npx_wrapper,
    ] + args_to_forward
    return subprocess.run(cmd, check=False)


def convert_results_for_resultdb_ingestion(
        test_output_filepath: str, isolated_output_filepath: str) -> None:
    # This is a very crude conversion, although the output of run-cts does not
    # give us a lot to work with. If the information it provides is ever
    # improved, this conversion can likely be substituted for native ResultDB
    # integration instead of relying on result_adapter.
    with open(test_output_filepath, encoding='utf-8') as infile:
        try:
            test_results = json.load(infile)
        except json.JSONDecodeError:
            logging.error(
                'Could not decode test output file. Tests likely did not run '
                'properly')
            test_results = [
                {
                    'TestCase': 'result_conversion',
                    'Status': 'fail',
                },
            ]

    converted_test_results = {
        'failures': [],
        'valid': True,
    }

    for r in test_results:
        if r['Status'] != 'pass':
            converted_test_results['failures'].append(r['TestCase'])

    with open(isolated_output_filepath, 'w', encoding='utf-8') as outfile:
        json.dump(converted_test_results, outfile)


def main() -> None:
    logging.getLogger().setLevel(logging.INFO)
    parser = argparse.ArgumentParser('Runs the WebGPU CTS via NodeJS')
    parser.add_argument(
        # We manually forward this argument so that we can use os.getcwd()
        # to work easily on the bots. Passing in "." will not work since
        # we change directories before invoking the underlying runner.
        '--output-directory',
        default=os.getcwd(),
        help='Output directory to use. Passed to the underlying runner as -bin.'
    )
    parser.add_argument('--isolated-script-test-output',
                        help='Path to the location to output JSON results.')
    parser.add_argument('--isolated-script-test-perf-output',
                        help='Currently unused, needed for bot support.')
    parser.add_argument('--isolated-script-test-launcher-retry-limit',
                        help='Currently unused, needed for bot support.')
    parser.add_argument('--isolated-script-test-repeat',
                        help='Currently unused, needed for bot support.')
    parser.add_argument('--isolated-script-test-filter',
                        help='Currently unused, needed for bot support.')
    args, unknown_args = parser.parse_known_args()

    with contextlib.chdir(CTS_DIR):
        node_helpers.add_node_to_path()
        install_npm_deps_in_current_dir()
        output_fd, output_filepath = tempfile.mkstemp()
        os.close(output_fd)
        try:
            proc = run_node_cts(args.output_directory, unknown_args,
                                output_filepath)
            if args.isolated_script_test_output:
                convert_results_for_resultdb_ingestion(
                    output_filepath, args.isolated_script_test_output)
            proc.check_returncode()
        finally:
            os.remove(output_filepath)


if __name__ == '__main__':
    main()
