#!/usr/bin/env python3
#
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
"""Merge script to generate/upload fuzz corpora for use in ClusterFuzz.

Note that this "merge" script does not actually merge any data - all Dawn
results are handled by ResultDB.
"""

import argparse
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile
from typing import List

DAWN_ROOT = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '..', '..'))
TESTING_MERGE_SCRIPTS = os.path.join(DAWN_ROOT, 'testing', 'merge_scripts')
sys.path.insert(0, TESTING_MERGE_SCRIPTS)

try:
    import merge_api
except ImportError as e:
    raise RuntimeError(
        'Unable to import merge_api - are you running in a Chromium checkout? '
        'This merge script only supports standalone Dawn checkouts.') from e

TRACE_SUBDIR = 'wire_traces'
FUZZER_NAMES = (
    'dawn_wire_server_and_frontend_fuzzer',
    'dawn_wire_server_and_vulkan_backend_fuzzer',
    'dawn_wire_server_and_d3d12_backend_fuzzer',
)
BUCKET = 'clusterfuzz-corpus'
BUCKET_DIRECTORY = 'libfuzzer'


def upload_files(input_directory: str) -> None:
    """Uploads all files in a directory to the ClusterFuzz bucket.

    One upload will be performed for each fuzzer in |FUZZER_NAMES|.

    Args:
        input_directory: A filepath to a directory whose contents will be
            uploaded.
    """
    gsutil_path = os.path.join(DAWN_ROOT, 'third_party', 'depot_tools',
                               'gsutil.py')
    if not os.path.exists(gsutil_path):
        raise RuntimeError(f'Unable to find gsutil.py at {gsutil_path}')

    for fn in FUZZER_NAMES:
        # This is effectively the same command line that would be run by the
        # older dawn/gn.py recipe for uploading the corpora when using the
        # gsutil recipe module.
        cmd = [
            sys.executable,
            '-u',  # Unbuffered output.
            gsutil_path,
            '-m',  # Multithreaded.
            '-o',  # Parallel upload.
            'GSUtil:parallel_composite_upload_threshold=50M',
            'cp',
            '-r',  # Recursive.
            '-n',  # No clobber.
            os.path.join(input_directory, '*'),
            f'gs://{BUCKET}/{BUCKET_DIRECTORY}/{fn}',
        ]
        p = subprocess.run(cmd)
        p.check_returncode()


def hash_trace_files(trace_files: List[str], output_directory: str) -> None:
    """Creates copies of trace files with hash-based names.

    Args:
        trace_files: A list of filepaths to trace files to process.
        output_directory: A filepath to a directory that the copies will be
            placed in.
    """
    for tf in trace_files:
        with open(tf, 'rb') as infile:
            digest = hashlib.md5(infile.read()).hexdigest()
        filename = os.path.join(output_directory, f'trace_{digest}')
        shutil.copyfile(tf, filename)


def find_trace_files(output_jsons: List[str]) -> List[str]:
    """Finds all wire trace files produced by the test.

    Args:
        output_jsons: A list of filepaths to the output.json files produced
            by each shard.

    Returns:
        A list of filepaths, one for each found trace file.
    """
    trace_files = []
    for json_file in output_jsons:
        isolated_outdir = os.path.dirname(json_file)
        dirname = os.path.join(isolated_outdir, TRACE_SUBDIR)
        for f in os.listdir(dirname):
            trace_files.append(os.path.join(dirname, f))
    if not trace_files:
        raise RuntimeError('Did not find any wire trace files')
    return trace_files


def generate_and_upload_fuzz_corpora(output_jsons: List[str]) -> None:
    """Generates and uploads fuzz corpora to ClusterFuzz.

    One corpus will be generated for each fuzzer in |FUZZER_NAMES|.

    Args:
        output_jsons: A list of filepaths to the output.json files produced
            by each shard.
    """
    trace_files = find_trace_files(output_jsons)
    with tempfile.TemporaryDirectory() as tempdir:
        hash_trace_files(trace_files, tempdir)
        upload_files(tempdir)


def main(cmdline_args: List[str]) -> None:
    parser = merge_api.ArgumentParser()
    args = parser.parse_args(cmdline_args)
    generate_and_upload_fuzz_corpora(args.jsons_to_merge)


if __name__ == '__main__':
    main(sys.argv[1:])
