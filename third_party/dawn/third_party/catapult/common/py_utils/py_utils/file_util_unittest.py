# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import errno
import os
import shutil
import tempfile
import unittest

from py_utils import file_util


class FileUtilTest(unittest.TestCase):

  def setUp(self):
    self._tempdir = tempfile.mkdtemp()

  def tearDown(self):
    shutil.rmtree(self._tempdir)

  def testCopySimple(self):
    source_path = os.path.join(self._tempdir, 'source')
    with open(source_path, 'w') as f:
      f.write('data')

    dest_path = os.path.join(self._tempdir, 'dest')

    self.assertFalse(os.path.exists(dest_path))
    file_util.CopyFileWithIntermediateDirectories(source_path, dest_path)
    self.assertTrue(os.path.exists(dest_path))
    self.assertEqual('data', open(dest_path, 'r').read())

  def testCopyMakeDirectories(self):
    source_path = os.path.join(self._tempdir, 'source')
    with open(source_path, 'w') as f:
      f.write('data')

    dest_path = os.path.join(self._tempdir, 'path', 'to', 'dest')

    self.assertFalse(os.path.exists(dest_path))
    file_util.CopyFileWithIntermediateDirectories(source_path, dest_path)
    self.assertTrue(os.path.exists(dest_path))
    self.assertEqual('data', open(dest_path, 'r').read())

  def testCopyOverwrites(self):
    source_path = os.path.join(self._tempdir, 'source')
    with open(source_path, 'w') as f:
      f.write('source_data')

    dest_path = os.path.join(self._tempdir, 'dest')
    with open(dest_path, 'w') as f:
      f.write('existing_data')

    file_util.CopyFileWithIntermediateDirectories(source_path, dest_path)
    self.assertEqual('source_data', open(dest_path, 'r').read())

  def testRaisesError(self):
    source_path = os.path.join(self._tempdir, 'source')
    with open(source_path, 'w') as f:
      f.write('data')

    dest_path = ""
    with self.assertRaises(OSError) as cm:
      file_util.CopyFileWithIntermediateDirectories(source_path, dest_path)
      self.assertEqual(errno.ENOENT, cm.exception.error_code)
