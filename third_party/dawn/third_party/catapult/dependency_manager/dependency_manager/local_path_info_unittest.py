# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

from pyfakefs import fake_filesystem_unittest

import dependency_manager

def _CreateFile(path):
  """Create file at specific |path|, with specific |content|."""
  with open(path, 'wb') as f:
    f.write(b'x')


def _ChangeFileTime(path, time0, days):
  new_time = time0 + (days * 24 * 60 * 60)
  os.utime(path, (new_time, new_time))


class LocalPathInfoTest(fake_filesystem_unittest.TestCase):

  def setUp(self):
    self.setUpPyfakefs()

  def tearDown(self):
    self.tearDownPyfakefs()

  def testEmptyInstance(self):
    path_info = dependency_manager.LocalPathInfo(None)
    self.assertIsNone(path_info.GetLocalPath())
    self.assertFalse(path_info.IsPathInLocalPaths('file.txt'))

  def testSimpleGroupWithOnePath(self):
    path_info = dependency_manager.LocalPathInfo(['file.txt'])
    self.assertTrue(path_info.IsPathInLocalPaths('file.txt'))
    self.assertFalse(path_info.IsPathInLocalPaths('other.txt'))

    # GetLocalPath returns None if the file doesn't exist.
    # Otherwise it will return the file path.
    self.assertIsNone(path_info.GetLocalPath())
    _CreateFile('file.txt')
    self.assertEqual('file.txt', path_info.GetLocalPath())

  def testSimpleGroupsWithMultiplePaths(self):
    path_info = dependency_manager.LocalPathInfo(
        [['file1', 'file2', 'file3']])
    self.assertTrue(path_info.IsPathInLocalPaths('file1'))
    self.assertTrue(path_info.IsPathInLocalPaths('file2'))
    self.assertTrue(path_info.IsPathInLocalPaths('file3'))

    _CreateFile('file1')
    _CreateFile('file2')
    _CreateFile('file3')
    s = os.stat('file1')
    time0 = s.st_mtime

    _ChangeFileTime('file1', time0, 4)
    _ChangeFileTime('file2', time0, 2)
    _ChangeFileTime('file3', time0, 0)
    self.assertEqual('file1', path_info.GetLocalPath())

    _ChangeFileTime('file1', time0, 0)
    _ChangeFileTime('file2', time0, 4)
    _ChangeFileTime('file3', time0, 2)
    self.assertEqual('file2', path_info.GetLocalPath())

    _ChangeFileTime('file1', time0, 2)
    _ChangeFileTime('file2', time0, 0)
    _ChangeFileTime('file3', time0, 4)
    self.assertEqual('file3', path_info.GetLocalPath())

  def testMultipleGroupsWithSinglePaths(self):
    path_info = dependency_manager.LocalPathInfo(
        ['file1', 'file2', 'file3'])
    self.assertTrue(path_info.IsPathInLocalPaths('file1'))
    self.assertTrue(path_info.IsPathInLocalPaths('file2'))
    self.assertTrue(path_info.IsPathInLocalPaths('file3'))

    self.assertIsNone(path_info.GetLocalPath())
    _CreateFile('file3')
    self.assertEqual('file3', path_info.GetLocalPath())
    _CreateFile('file2')
    self.assertEqual('file2', path_info.GetLocalPath())
    _CreateFile('file1')
    self.assertEqual('file1', path_info.GetLocalPath())

  def testMultipleGroupsWithMultiplePaths(self):
    path_info = dependency_manager.LocalPathInfo([
        ['file1', 'file2'],
        ['file3', 'file4']])
    self.assertTrue(path_info.IsPathInLocalPaths('file1'))
    self.assertTrue(path_info.IsPathInLocalPaths('file2'))
    self.assertTrue(path_info.IsPathInLocalPaths('file3'))
    self.assertTrue(path_info.IsPathInLocalPaths('file4'))

    _CreateFile('file1')
    _CreateFile('file3')
    s = os.stat('file1')
    time0 = s.st_mtime

    # Check that file1 is always returned, even if it is not the most recent
    # file, because it is part of the first group and exists.
    _ChangeFileTime('file1', time0, 2)
    _ChangeFileTime('file3', time0, 0)
    self.assertEqual('file1', path_info.GetLocalPath())

    _ChangeFileTime('file1', time0, 0)
    _ChangeFileTime('file3', time0, 2)
    self.assertEqual('file1', path_info.GetLocalPath())

  def testUpdate(self):
    path_info1 = dependency_manager.LocalPathInfo(
        [['file1', 'file2']])  # One group with two files.
    path_info2 = dependency_manager.LocalPathInfo(
        ['file1', 'file2', 'file3'])  # Three groups
    self.assertTrue(path_info1.IsPathInLocalPaths('file1'))
    self.assertTrue(path_info1.IsPathInLocalPaths('file2'))
    self.assertFalse(path_info1.IsPathInLocalPaths('file3'))

    _CreateFile('file3')
    self.assertIsNone(path_info1.GetLocalPath())

    path_info1.Update(path_info2)
    self.assertTrue(path_info1.IsPathInLocalPaths('file1'))
    self.assertTrue(path_info1.IsPathInLocalPaths('file2'))
    self.assertTrue(path_info1.IsPathInLocalPaths('file3'))
    self.assertEqual('file3', path_info1.GetLocalPath())

    _CreateFile('file1')
    time0 = os.stat('file1').st_mtime
    _ChangeFileTime('file3', time0, 2)  # Make file3 more recent.

    # Check that file3 is in a later group.
    self.assertEqual('file1', path_info1.GetLocalPath())
