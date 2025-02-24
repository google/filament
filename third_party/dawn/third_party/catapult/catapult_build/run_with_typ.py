#!/usr/bin/env vpython3
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A wrapper around typ (test your projects)."""

import os
import sys


def Run(top_level_dir, path=None, suffixes=None, **kwargs):
  """Runs a set of Python tests using typ.

  Args:
    top_level_dir: Directory to look for Python unit tests in.
    path: A list of extra paths to add to sys.path when running the tests.

  Returns:
    An exit code (0 for success, otherwise non-zero).
  """
  if not suffixes:
    suffixes = ['*_test.py', '*_unittest.py']
  typ_path = os.path.abspath(os.path.join(
      os.path.dirname(__file__), os.path.pardir, 'third_party', 'typ'))
  _AddToPathIfNeeded(typ_path)
  import typ # pylint: disable=import-outside-toplevel
  return typ.main(
      top_level_dir=top_level_dir,
      path=(path or []),
      coverage_source=[top_level_dir],
      suffixes=suffixes,
      **kwargs)


def _AddToPathIfNeeded(path):
  if path not in sys.path:
    sys.path.insert(0, path)
