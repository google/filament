# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import errno
import os
import shutil


def CopyFileWithIntermediateDirectories(source_path, dest_path):
  """Copies a file and creates intermediate directories as needed.

  Args:
    source_path: Path to the source file.
    dest_path: Path to the destination where the source file should be copied.
  """
  assert os.path.exists(source_path)
  try:
    os.makedirs(os.path.dirname(dest_path))
  except OSError as e:
    if e.errno != errno.EEXIST:
      raise
  shutil.copy(source_path, dest_path)
