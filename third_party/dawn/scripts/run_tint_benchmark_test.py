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
"""Runs the Tint benchmark to check that it works.

This script is necessary in order to support running the Tint benchmark on Swarming, but it
is effectively a thin wrapper around the underlying tint_benchmark binary.
"""

import argparse
import json
import os
import subprocess


def run_benchmark() -> subprocess.CompletedProcess:
    cmd = [
        os.path.join(os.getcwd(), 'tint_benchmark'),
        '--benchmark_dry_run',
    ]
    return subprocess.run(cmd, check=False)


def generate_results_for_resultdb_ingestion(success: bool,
                                            output_file: str) -> None:
    # This is a very crude pass/fail result since we just want to know if the
    # benchmark is able to run without crashing.
    generated_test_results = {
        'failures': [],
        'valid': True,
    }
    if not success:
        generated_test_results['failures'].append('tint_benchmark')

    with open(output_file, 'w', encoding='utf-8') as outfile:
        json.dump(generated_test_results, outfile)


def main() -> None:
    parser = argparse.ArgumentParser(description='Runs the Tint benchmark')

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
    args = parser.parse_args()

    proc = run_benchmark()
    if args.isolated_script_test_output:
        generate_results_for_resultdb_ingestion(
            not proc.returncode, args.isolated_script_test_output)
    proc.check_returncode()


if __name__ == '__main__':
    main()
