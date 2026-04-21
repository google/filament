# Copyright (C) 2026 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!/usr/bin/env python3

import argparse
import json
import os
import sys
import urllib.request
import urllib.error
import subprocess

# The base URL where historical size data is stored
BASE_URL = "https://raw.githubusercontent.com/google/filament-assets/main/sizeguard/"

def get_merge_base(target_branch="origin/main"):
  """Finds the merge base between HEAD and the target branch."""
  try:
    # Fetch the target branch to ensure we have the reference
    result = subprocess.run(
      ["git", "merge-base", "HEAD", target_branch],
      stdout=subprocess.PIPE,
      stderr=subprocess.PIPE,
      check=True,
      text=True
    )
    return result.stdout.strip()
  except subprocess.CalledProcessError:
    print(f"Warning: Could not determine merge base with {target_branch}.", file=sys.stderr)
    return None

def get_ancestors(start_commit, count=50):
  """Returns a list of ancestor commit hashes."""
  try:
    result = subprocess.run(
      ["git", "rev-list", f"--max-count={count}", start_commit],
      stdout=subprocess.PIPE,
      stderr=subprocess.PIPE,
      check=True,
      text=True
    )
    return result.stdout.strip().splitlines()
  except subprocess.CalledProcessError as e:
    print(f"Error listing ancestors: {e}", file=sys.stderr)
    return []

def fetch_json(commit_hash):
  """Fetches the JSON file for the given commit from the assets repo."""
  url = f"{BASE_URL}{commit_hash}.json"
  try:
    with urllib.request.urlopen(url) as response:
      if response.status == 200:
        print(f"Found historical data for commit {commit_hash}")
        return json.loads(response.read().decode('utf-8'))
  except urllib.error.HTTPError as e:
    if e.code == 404:
      return None
    print(f"Warning: HTTP error fetching {url}: {e}", file=sys.stderr)
  except Exception as e:
    print(f"Warning: Error fetching {url}: {e}", file=sys.stderr)
  return None

def flatten_data(data):
  """Flattens the JSON structure into a dictionary of name -> size."""
  flat = {}
  for item in data:
    # Top level archive
    flat[item['name']] = item['size']
    # Inner content
    if 'content' in item:
      for inner in item['content']:
        # Key format: ArchiveName/InnerName
        key = f"{item['name']}/{inner['name']}"
        flat[key] = inner['size']
  return flat

def main():
  parser = argparse.ArgumentParser(
    description="Compare current artifact sizes against historical data."
  )
  parser.add_argument(
    "current_json", help="Path to the JSON file generated for the current build."
  )
  parser.add_argument(
    "--threshold", type=int, default=20480, help="Size increase threshold in bytes (default: 20KB)."
  )
  parser.add_argument(
    "--target-branch", default="origin/main", help="The branch to compare against."
  )
  parser.add_argument(
    "--artifacts", nargs="+",
    help="List of artifact paths to check (e.g. 'foo.aar' or 'foo.aar/lib/arm64/bar.so')."
  )
  parser.add_argument(
    "--bypass", action="store_true",
    help="Bypass the size threshold check and exit successfully."
  )

  args = parser.parse_args()

  if not os.path.exists(args.current_json):
    print(f"Error: Current JSON file not found: {args.current_json}", file=sys.stderr)
    sys.exit(1)

  with open(args.current_json, 'r') as f:
    current_data = json.load(f)

  # 1. Find the starting commit (merge base)
  start_commit = get_merge_base(args.target_branch)
  if not start_commit:
    print("Error: Could not determine a valid starting commit to search.", file=sys.stderr)
    sys.exit(1)

  print(f"Merge base with {args.target_branch} is {start_commit}")

  # 2. Search for historical data
  ancestors = get_ancestors(start_commit)
  base_data = None
  base_commit = None

  for commit in ancestors:
    base_data = fetch_json(commit)
    if base_data:
      base_commit = commit
      break

  if not base_data:
    print(f"Warning: No historical size data found in the last {len(ancestors)} ancestors.",
          file=sys.stderr)
    sys.exit(0)

  print(f"Comparing against historical data from commit {base_commit}")

  # 3. Compare
  current_flat = flatten_data(current_data)
  base_flat = flatten_data(base_data)

  failures = []
  checked_count = 0

  print(f"{'Artifact':<60} | {'Current':<10} | {'Base':<10} | {'Delta':<10} | {'Status'}")
  print("-" * 110)

  keys_to_check = args.artifacts if args.artifacts else current_flat.keys()

  for name in keys_to_check:
    if name not in current_flat:
      print(f"Warning: Artifact '{name}' not found in current build output.", file=sys.stderr)
      continue

    current_size = current_flat[name]
    checked_count += 1

    status = "OK"
    base_str = "N/A"
    diff_str = "N/A"

    if name in base_flat:
      base_size = base_flat[name]
      base_str = str(base_size)
      diff = current_size - base_size
      diff_str = f"{diff:+}"

      if diff > args.threshold:
        failures.append(name)
        status = "FAIL"
    else:
      status = "NEW"

    print(f"{name:<60} | {current_size:<10} | {base_str:<10} | {diff_str:<10} | {status}")

  print("-" * 110)

  if args.bypass:
    print("SUCCESS: Size guard test has been bypassed via commit message tag.")
    sys.exit(0)
  elif failures:
    print(f"FAILURE: {len(failures)} artifacts exceeded threshold of {args.threshold} bytes.")
    sys.exit(1)
  else:
    print(f"SUCCESS: {checked_count} artifacts checked. All within acceptable threshold.")
    sys.exit(0)

if __name__ == "__main__":
  main()
