#!/usr/bin/env python3
# Copyright 2026 The Dawn & Tint Authors
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
"""Runs the LiteRT-LM performance tests.

This script acts as a thin wrapper around the litert_lm_advanced_main binary
to support running it on Swarming with correct libraries and paths.
"""

import argparse
import json
import os
from pathlib import Path
import subprocess


def run_litert_lm(metric_proto_file_path: Path) -> subprocess.CompletedProcess:
    repo_root = Path(__file__).resolve().parent.parent
    binary_path = Path.cwd() / 'litert_lm_advanced_main'
    model_path = repo_root / 'third_party' / 'litert-lm' / 'data' / 'model.litertlm'

    cmd = [
        str(binary_path),
        '--benchmark',
        '--backend=gpu',
        f'--model_path={model_path}',
        f'--metric_proto_file_path={metric_proto_file_path}',
    ]

    print(f"Executing: {' '.join(cmd)}")

    # The .so files are isolated alongside the binary in the current build out directory,
    # so explicitly add them to the library path for execution on Linux bots.
    env = os.environ.copy()
    env['LD_LIBRARY_PATH'] = f"{Path.cwd()}{os.pathsep}{env.get('LD_LIBRARY_PATH', '')}"

    return subprocess.run(cmd, env=env, check=False)


def generate_results_for_resultdb_ingestion(success: bool,
                                            output_file: str) -> None:
    generated_test_results = {
        'failures': [],
        'valid': True,
    }
    if not success:
        generated_test_results['failures'].append('litert_lm_advanced_main')

    with open(output_file, 'w', encoding='utf-8') as outfile:
        json.dump(generated_test_results, outfile)


def main() -> None:
    parser = argparse.ArgumentParser(description='Runs LiteRT-LM benchmarks')

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

    # Determine output directory for results.
    if args.isolated_script_test_output:
        output_dir = Path(args.isolated_script_test_output).parent
    else:
        output_dir = Path.cwd()

    # Run the benchmark and generate metric results file.
    metric_file = output_dir / 'litert_lm_metrics.pb'
    proc = run_litert_lm(metric_file)
    if args.isolated_script_test_output:
        generate_results_for_resultdb_ingestion(
            not proc.returncode, args.isolated_script_test_output)
    proc.check_returncode()


if __name__ == '__main__':
    main()
