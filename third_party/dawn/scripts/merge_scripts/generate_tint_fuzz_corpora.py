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
"""Merge script to upload Tint fuzz corpora for use in ClusterFuzz.

Note that this "merge" script does not actually merge any data - all Dawn
results are handled by ResultDB.
"""

import os
import tempfile
import shutil
import sys

ISOLATED_OUT_SUBDIR = 'clusterfuzz'
DAWN_ROOT = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, DAWN_ROOT)

from scripts.merge_scripts import fuzz_corpora_common

try:
    from testing.merge_scripts import merge_api
except ImportError as e:
    raise RuntimeError(
        'Unable to import merge_api - are you running in a Chromium checkout? '
        'This merge script only supports standalone Dawn checkouts.') from e


def main() -> None:
    parser = merge_api.ArgumentParser()
    fuzz_corpora_common.add_common_arguments(parser)
    args = parser.parse_args()
    # Tint tests upload their files directly with clobbering. This is in
    # contrast to the wire trace tests, which use hash-based file names and do
    # not clobber.
    trace_files = fuzz_corpora_common.find_raw_trace_files(
        args.jsons_to_merge, ISOLATED_OUT_SUBDIR)
    with tempfile.TemporaryDirectory() as tempdir:
        for tf in trace_files:
            shutil.copy(tf, tempdir)
        for fn in args.fuzzer_names:
            fuzz_corpora_common.upload_directory_to_gcs(tempdir,
                                                        fn,
                                                        clobber=False)


if __name__ == '__main__':
    main()
