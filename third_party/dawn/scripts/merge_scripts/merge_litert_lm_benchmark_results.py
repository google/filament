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
"""Merge script to aggregate LiteRT-LM benchmark results and upload metrics to GCS.
"""

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

DAWN_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(DAWN_ROOT))

from testing.merge_scripts import merge_api


def find_gsutil() -> list[str]:
    gsutil_path = DAWN_ROOT / 'third_party' / 'depot_tools' / 'gsutil.py'
    if not gsutil_path.exists():
        raise RuntimeError(f'Unable to find gsutil.py at {gsutil_path}')
    return [sys.executable, '-u', str(gsutil_path)]


def upload_file_to_gcs(local_file_path: Path, gcs_bucket: str,
                       gcs_dest_path: str) -> None:
    if not local_file_path.exists():
        raise FileNotFoundError(f"Local file not found at: {local_file_path}")

    gsutil_cmd = find_gsutil()
    gcs_url = f"gs://{gcs_bucket}/{gcs_dest_path}"

    cmd = gsutil_cmd + ['cp', str(local_file_path), gcs_url]
    print(f"Uploading {local_file_path.name} to GCS: {' '.join(cmd)}")
    subprocess.run(cmd, check=True)


def generate_metadata(metadata_file_path: Path, timestamp: int,
                      build_properties_str: str | None) -> None:
    props = {}
    if build_properties_str:
        try:
            props = json.loads(build_properties_str)
        except Exception as e:
            print(f"Warning: Failed to parse build-properties: {e}",
                  file=sys.stderr)

    metadata = {
        'timestamp': timestamp,
        'git_revision': props.get('got_revision'),
        'buildername': props.get('buildername'),
        'builder_group': props.get('builder_group'),
    }

    with open(metadata_file_path, 'w', encoding='utf-8') as f:
        json.dump(metadata, f, indent=2)


def main() -> int:
    parser = merge_api.ArgumentParser()
    args = parser.parse_args()

    # This benchmark is designed to run only on a single shard.
    # Raise an error if more than one shard output is passed to be merged.
    if len(args.jsons_to_merge) > 1:
        raise ValueError(
            f"Expected exactly 1 shard for litert_lm_benchmark, "
            f"but found {len(args.jsons_to_merge)} shards: {args.jsons_to_merge}"
        )

    # Merge standard test result JSONs (for exactly 0 or 1 shard).
    merged_results = {
        'failures': [],
        'valid': True,
    }
    for json_file_str in args.jsons_to_merge:
        json_file = Path(json_file_str)
        if not json_file.exists():
            continue
        try:
            with open(json_file, 'r', encoding='utf-8') as f:
                results = json.load(f)
            if results.get('failures'):
                merged_results['failures'].extend(results['failures'])
            if not results.get('valid', True):
                merged_results['valid'] = False
        except Exception as e:
            print(f"Error reading {json_file}: {e}", file=sys.stderr)
            merged_results['valid'] = False

    # Write the combined results JSON required by the merge API.
    output_json_path = Path(args.output_json)
    with open(output_json_path, 'w', encoding='utf-8') as f:
        json.dump(merged_results, f, indent=2)

    # Locate litert_lm_metrics.pb from the single shard output.
    metric_file = None
    if args.jsons_to_merge:
        json_file = Path(args.jsons_to_merge[0])
        isolated_outdir = json_file.parent
        pb_path = isolated_outdir / 'litert_lm_metrics.pb'
        if pb_path.exists():
            metric_file = pb_path

    if not metric_file:
        print("Warning: No litert_lm_metrics.pb file found to upload.",
              file=sys.stderr)
        return 0

    # Upload metrics and generated metadata to GCS.
    timestamp = int(time.time())
    task_id = os.environ.get('SWARMING_TASK_ID', 'local')
    run_id = f"{timestamp}_{task_id}"
    bucket_name = 'dawn-webgpu-perf-results'
    directory_name = f'litert_lm_benchmark/{run_id}'

    with tempfile.TemporaryDirectory() as tempdir_str:
        tempdir = Path(tempdir_str)
        # Generate metadata.json locally with accurate builder context.
        metadata_file = tempdir / 'metadata.json'
        generate_metadata(metadata_file, timestamp,
                          getattr(args, 'build_properties', None))

        upload_file_to_gcs(metadata_file, bucket_name,
                           f"{directory_name}/metadata.json")

        # Upload the single metric file.
        upload_file_to_gcs(metric_file, bucket_name,
                           f"{directory_name}/litert_lm_metrics.pb")

    return 0


if __name__ == '__main__':
    sys.exit(main())
