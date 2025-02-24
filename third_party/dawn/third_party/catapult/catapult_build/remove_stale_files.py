#!/usr/bin/env python
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import sys


def RemoveAllStaleFiles(base_dir, types_to_purge):
  """Scan directories for old files and delete them."""
  for type_ext in types_to_purge:
    for dirname, _, filenames in os.walk(base_dir):
      if '.git' in dirname:
        continue
      for filename in filenames:
        _, ext = os.path.splitext(filename)
        if ext != type_ext:
          continue

        full_path = os.path.join(dirname, filename)

        try:
          os.remove(full_path)
        except OSError:
          # Wrap OS calls in try/except in case another process touched this
          # file.
          pass

      try:
        os.removedirs(dirname)
      except OSError:
        # Wrap OS calls in try/except in case another process touched this dir.
        pass


def Main():
  parser = argparse.ArgumentParser()
  parser.add_argument('path', help='Path to clear stale files from.')
  parser.add_argument('types', help='Comma separated list of types to purge.')
  args = parser.parse_args(sys.argv[1:])

  RemoveAllStaleFiles(args.path, args.types.split(','))


if __name__ == '__main__':
  Main()
