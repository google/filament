#!/usr/bin/env python3
# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
This script finds the SetEnv.*.json file from an unpacked Windows SDK.
"""

import argparse
import os
import sys

POSSIBLE_BASE_LOCATIONS = [
    # Older SDKs just used the raw 'bin' directory
    os.path.join("win_sdk", "bin"),

    # SDK versions after 19041
    os.path.join("Windows Kits", "10", "bin"),
]

SDK_ARCHITECTURES = frozenset([
    "x86",
    "x64",
    "arm64",
])


def main():
  parser = argparse.ArgumentParser(
      description='Find the SetEnv from a windows SDK')
  parser.add_argument('--sdk_root',
                      metavar='PATH',
                      required=True,
                      help='The absolute path to the root of the unpacked SDK')
  parser.add_argument('--target_arch',
                      choices=SDK_ARCHITECTURES,
                      required=True,
                      help='The target architecture')
  parser.add_argument('--output_json',
                      required=True,
                      help='The absolute path to an output json file')
  args = parser.parse_args()

  if not os.path.isabs(args.sdk_root):
    parser.error("sdk_root must be absolute, got {!r}".format(args.sdk_root))

  if not os.path.isabs(args.output_json):
    parser.error("output_json must be absolute, got {!r}".format(
        args.output_json))

  with open(args.output_json, 'w', encoding="utf-8") as outf:
    tail = "SetEnv.{}.json".format(args.target_arch)
    for loc in POSSIBLE_BASE_LOCATIONS:
      full = os.path.join(args.sdk_root, loc, tail)
      print("Attempting:", full)
      try:
        with open(full, encoding="utf-8") as set_env_json:
          print(">  Found it!")
          outf.write(set_env_json.read())
          return
      except OSError as ex:
        print(">  Failed:", ex)
        continue

  print("Unable to locate SetEnv file")
  sys.exit(1)


if __name__ == '__main__':
  main()
