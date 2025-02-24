# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import unittest
import zipfile

from devil import devil_env
from devil.utils import zip_utils

with devil_env.SysPath(devil_env.PY_UTILS_PATH):
  from py_utils import tempfile_ext


class WriteZipFileTest(unittest.TestCase):
  def testSimple(self):
    with tempfile_ext.NamedTemporaryDirectory() as working_dir:
      file1 = os.path.join(working_dir, 'file1.txt')
      file2 = os.path.join(working_dir, 'file2.txt')

      with open(file1, 'w') as f1:
        f1.write('file1')
      with open(file2, 'w') as f2:
        f2.write('file2')

      zip_tuples = [
          (file1, 'foo/file1.txt'),
          (file2, 'bar/file2.txt'),
      ]

      zip_path = os.path.join(working_dir, 'out.zip')
      zip_utils.WriteZipFile(zip_path, zip_tuples)

      self.assertTrue(zipfile.is_zipfile(zip_path))

      actual = zipfile.ZipFile(zip_path)
      expected_files = [
          'foo/file1.txt',
          'bar/file2.txt',
      ]

      self.assertEqual(sorted(expected_files), sorted(actual.namelist()))
