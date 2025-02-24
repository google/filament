#!/usr/bin/env python3
# Copyright (c) 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Small utility script to enable/disable `depot_tools` automatic updating."""

import argparse
import datetime
import os
import sys

DEPOT_TOOLS_ROOT = os.path.abspath(os.path.dirname(__file__))
SENTINEL_PATH = os.path.join(DEPOT_TOOLS_ROOT, '.disable_auto_update')


def main():
    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--enable',
                       action='store_true',
                       help='Enable auto-updating.')
    group.add_argument('--disable',
                       action='store_true',
                       help='Disable auto-updating.')
    args = parser.parse_args()

    if args.enable:
        if os.path.exists(SENTINEL_PATH):
            os.unlink(SENTINEL_PATH)
    if args.disable:
        if not os.path.exists(SENTINEL_PATH):
            with open(SENTINEL_PATH, 'w') as fd:
                fd.write('Disabled by %s at %s\n' %
                         (__file__, datetime.datetime.now()))
    return 0


if __name__ == '__main__':
    sys.exit(main())
