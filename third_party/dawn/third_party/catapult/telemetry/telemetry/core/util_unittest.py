# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import os
import shutil
import tempfile
import unittest
from unittest import mock

from telemetry.core import util


class TestGetSequentialFileName(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self.test_directory = None

  def setUp(self):
    self.test_directory = tempfile.mkdtemp()

  def testGetSequentialFileNameNoOtherSequentialFile(self):
    next_json_test_file_path = util.GetSequentialFileName(os.path.join(
        self.test_directory, 'test'))
    self.assertEqual(os.path.join(self.test_directory, 'test_000'),
                     next_json_test_file_path)

  def testGetSequentialFileNameWithOtherSequentialFiles(self):
    # Create test_000.json, test_001.json, test_002.json in test directory.
    for i in range(3):
      with open(
          os.path.join(self.test_directory, 'test_%03d.json' % i), 'w') as _:
        pass
    next_json_test_file_path = util.GetSequentialFileName(os.path.join(
        self.test_directory, 'test'))
    self.assertEqual(os.path.join(self.test_directory, 'test_003'),
                     next_json_test_file_path)

  def tearDown(self):
    shutil.rmtree(self.test_directory)


class TestGetUsedBuildDirectory(unittest.TestCase):

  def testGetUsedBuildDirectoryBrowserDirectoryExists(self):
    with mock.patch('os.path.exists') as m:
      m.return_value = True
      self.assertEqual(util.GetUsedBuildDirectory('/foo/test'), '/foo/test')

  def testGetUsedBuildDirectoryBrowserDirectoryDoesNotExist(self):
    with mock.patch('os.path.exists') as m:
      m.return_value = False
      self.assertNotEqual(util.GetUsedBuildDirectory('/foo/test'), '/foo/test')

  # Patched so that CHROMIUM_OUTPUT_DIR doesn't affect the test if it's set.
  @mock.patch.dict(os.environ, {}, clear=True)
  def testGetUsedBuildDirectoryNoBrowserDirectory(self):
    def side_effect(arg):
      return arg == os.path.join('.', 'out', 'Release_x64')

    with mock.patch('os.path.exists') as m:
      m.side_effect = side_effect
      self.assertEqual(util.GetUsedBuildDirectory(chrome_root='.'),
                       os.path.join('.', 'out', 'Release_x64'))


class TestGetBuildDirFromHostApkPath(unittest.TestCase):
  def testNoPathReturnsNone(self):
    self.assertEqual(util.GetBuildDirFromHostApkPath(None), None)

  def testLocallyBuiltPaths(self):
    self.assertEqual(util.GetBuildDirFromHostApkPath('/out/Foo/apks/test.apk'),
                     '/out/Foo')
    self.assertEqual(
        util.GetBuildDirFromHostApkPath('/out/Bar/bin/test_bundle'), '/out/Bar')

  def testNonLocallyBuiltPath(self):
    self.assertEqual(
        util.GetBuildDirFromHostApkPath('/some/other/path/test.apk'), None)
