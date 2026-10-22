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
"""Common code for ClusterFuzz corpora generation/uploading."""

import argparse
import hashlib
import os
import shutil
import subprocess
import sys

DAWN_ROOT = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '..', '..'))

BUCKET = 'clusterfuzz-corpus'
BUCKET_DIRECTORY = 'libfuzzer'


def upload_directory_to_gcs(local_directory: str, fuzzer_name: str,
                            clobber: bool) -> None:
    """Uploads the contents of a directory to the ClusterFuzz GCS bucket.

    Args:
        local_directory: The path to the local directory whose contents will
            be uploaded.
        fuzzer_name: The ClusterFuzz fuzzer name to upload the files under.
        clobber: Whether to clobber identically named files in the GCS bucket.
    """
    gsutil_path = os.path.join(DAWN_ROOT, 'third_party', 'depot_tools',
                               'gsutil.py')
    if not os.path.exists(gsutil_path):
        raise RuntimeError(f'Unable to find gsutil.py at {gsutil_path}')

    cmd = [
        sys.executable,
        '-u',  # Unbuffered output.
        gsutil_path,
        '-m',  # Multithreaded.
        '-o',  # Parallel upload.
        'GSUtil:parallel_composite_upload_threshold=50M',
        'cp',
        '-r',  # Recursive.
    ]
    if not clobber:
        cmd.append('-n')
    cmd.extend([
        os.path.join(local_directory, '*'),
        f'gs://{BUCKET}/{BUCKET_DIRECTORY}/{fuzzer_name}',
    ])
    p = subprocess.run(cmd, check=True)


def hash_trace_files(trace_files: list[str], output_directory: str) -> None:
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


def find_raw_trace_files(output_jsons: list[str],
                         subdirectory: str) -> list[str]:
    """Finds all raw trace files produced by the test.

    Args:
        output_jsons: A list of filepaths to the output.json files produced
            by each shard.
        subdirectory: The subdirectory of the isolated output directory that
            is expected to contain the raw trace files.

    Returns:
        A list of filepaths, one for each found trace file.
    """
    trace_files = []
    for json_file in output_jsons:
        isolated_outdir = os.path.dirname(json_file)
        dirname = os.path.join(isolated_outdir, subdirectory)
        for f in os.listdir(dirname):
            trace_files.append(os.path.join(dirname, f))
    if not trace_files:
        raise RuntimeError('Did not find any wire trace files')
    return trace_files


def add_common_arguments(parser: argparse.ArgumentParser) -> None:
    """Adds common fuzz corpora-related arguments to a parser.

    Args:
        parser: The ArgumentParser to add arguments to.
    """
    parser.add_argument(
        '--fuzzer-name',
        required=True,
        dest='fuzzer_names',
        action='append',
        help=('A fuzzer name to upload files to. Can be specified multiple '
              'times'))
