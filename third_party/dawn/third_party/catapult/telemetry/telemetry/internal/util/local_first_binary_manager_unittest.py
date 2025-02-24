# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import shutil
import tempfile
import unittest
from unittest import mock

from telemetry.core import exceptions
from telemetry.internal.util import binary_manager
from telemetry.internal.util import local_first_binary_manager


def ClearBinaryManagerInstance(test):
  test.previous_manager_instance =\
      local_first_binary_manager.LocalFirstBinaryManager._instance
  local_first_binary_manager.LocalFirstBinaryManager._instance = None


def RestoreBinaryManagerInstance(test):
  local_first_binary_manager.LocalFirstBinaryManager._instance =\
      test.previous_manager_instance


class LocalFirstBinaryManagerFetchPathTest(unittest.TestCase):
  def setUp(self):
    ClearBinaryManagerInstance(self)

  def tearDown(self):
    RestoreBinaryManagerInstance(self)

  @mock.patch.object(local_first_binary_manager.LocalFirstBinaryManager,
                     '_FetchBinaryManagerPath')
  @mock.patch.object(local_first_binary_manager.LocalFirstBinaryManager,
                     '_FetchLocalPath')
  def testLocalPathReturned(self, local_mock, remote_mock):
    local_mock.return_value = 'path'
    bm = local_first_binary_manager.LocalFirstBinaryManager(
        None, None, None, None, None, None)
    path = bm.FetchPath('dep')
    self.assertEqual(path, 'path')
    local_mock.assert_called_once_with('dep')
    # mock's built-in call counting appears to be broken, as the following will
    # pass at the time of writing:
    #   local_mock.assert_not_called()
    #   local_mock.assert_called_once()
    # However, manually checking the call counts works, so use that.
    self.assertEqual(local_mock.call_count, 1)
    self.assertEqual(remote_mock.call_count, 0)

  @mock.patch.object(local_first_binary_manager.LocalFirstBinaryManager,
                     '_FetchBinaryManagerPath')
  @mock.patch.object(local_first_binary_manager.LocalFirstBinaryManager,
                     '_FetchLocalPath')
  def testRemotePathReturned(self, local_mock, remote_mock):
    local_mock.return_value = None
    remote_mock.return_value = 'path'
    bm = local_first_binary_manager.LocalFirstBinaryManager(
        None, None, None, None, None, None)
    path = bm.FetchPath('dep')
    self.assertEqual(path, 'path')
    local_mock.assert_called_once_with('dep')
    remote_mock.assert_called_once_with('dep')
    self.assertEqual(local_mock.call_count, 1)
    self.assertEqual(remote_mock.call_count, 1)

  @mock.patch.object(local_first_binary_manager.LocalFirstBinaryManager,
                     '_FetchBinaryManagerPath')
  @mock.patch.object(local_first_binary_manager.LocalFirstBinaryManager,
                     '_FetchLocalPath')
  def testRemotePathException(self, local_mock, remote_mock):
    def RaiseException(_):
      raise binary_manager.NoPathFoundError(None, None)
    local_mock.return_value = None
    remote_mock.side_effect = RaiseException
    bm = local_first_binary_manager.LocalFirstBinaryManager(
        None, None, None, None, None, None)
    path = bm.FetchPath('dep')
    self.assertEqual(path, None)
    # Ensure the value is cached.
    self.assertIn('dep', bm._dependency_cache)
    local_mock.assert_called_once_with('dep')
    remote_mock.assert_called_once_with('dep')
    self.assertEqual(local_mock.call_count, 1)
    self.assertEqual(remote_mock.call_count, 1)


class LocalFirstBinaryManagerFetchLocalPathTest(unittest.TestCase):
  def setUp(self):
    ClearBinaryManagerInstance(self)
    self._build_dir = tempfile.mkdtemp()

  def tearDown(self):
    RestoreBinaryManagerInstance(self)
    shutil.rmtree(self._build_dir)
    self._build_dir = None

  def testNoBuildDirectory(self):
    bm = local_first_binary_manager.LocalFirstBinaryManager(
        None, None, None, None, None, None)
    self.assertIsNone(bm._FetchLocalPath('dep'))

  def testIgnoredDependency(self):
    bm = local_first_binary_manager.LocalFirstBinaryManager(
        self._build_dir, None, None, None, ['ignored_dep'], None)
    open(os.path.join(self._build_dir, 'ignored_dep'), 'w').close()
    self.assertIsNone(bm._FetchLocalPath('ignored_dep'))

  def testMissingDependency(self):
    bm = local_first_binary_manager.LocalFirstBinaryManager(
        self._build_dir, None, None, None, [], None)
    self.assertIsNone(bm._FetchLocalPath('dep'))

  def testValidDependency(self):
    bm = local_first_binary_manager.LocalFirstBinaryManager(
        self._build_dir, None, None, None, [], None)
    dep_path = os.path.join(self._build_dir, 'dep')
    open(dep_path, 'w').close()
    self.assertEqual(bm._FetchLocalPath('dep'), dep_path)


class LocalFirstBinaryManagerFetchBinaryManagerPathTest(unittest.TestCase):
  def setUp(self):
    ClearBinaryManagerInstance(self)

  def tearDown(self):
    RestoreBinaryManagerInstance(self)

  @mock.patch('telemetry.internal.util.binary_manager.FetchPath')
  @mock.patch('telemetry.internal.util.binary_manager.NeedsInit')
  def testBinaryManagerPathReturned(self, needs_init_mock, fetch_mock):
    needs_init_mock.return_value = False
    fetch_mock.return_value = 'path'
    bm = local_first_binary_manager.LocalFirstBinaryManager(
        None, None, 'os', 'arch', None, 'os_version')
    self.assertEqual(bm._FetchBinaryManagerPath('dep'), 'path')
    self.assertEqual(needs_init_mock.call_count, 1)
    self.assertEqual(fetch_mock.call_count, 1)
    fetch_mock.assert_called_once_with('dep', 'os', 'arch', 'os_version')

  @mock.patch('telemetry.internal.util.binary_manager.FetchPath')
  @mock.patch('telemetry.internal.util.binary_manager.InitDependencyManager')
  @mock.patch('telemetry.internal.util.binary_manager.NeedsInit')
  def testBinaryManagerInitialized(
      self, needs_init_mock, init_mock, _):
    needs_init_mock.return_value = False
    bm = local_first_binary_manager.LocalFirstBinaryManager(
        None, None, None, None, None, None)
    bm._FetchBinaryManagerPath(None)
    self.assertEqual(needs_init_mock.call_count, 1)
    self.assertEqual(init_mock.call_count, 0)

  @mock.patch('telemetry.internal.util.binary_manager.FetchPath')
  @mock.patch('telemetry.internal.util.binary_manager.InitDependencyManager')
  @mock.patch('telemetry.internal.util.binary_manager.NeedsInit')
  def testBinaryManagerUninitialized(
      self, needs_init_mock, init_mock, _):
    needs_init_mock.return_value = True
    bm = local_first_binary_manager.LocalFirstBinaryManager(
        None, None, None, None, None, None)
    bm._FetchBinaryManagerPath(None)
    self.assertEqual(needs_init_mock.call_count, 1)
    self.assertEqual(init_mock.call_count, 1)
    init_mock.assert_called_once_with(None)


class LocalFirstBinaryManagerInitializationTest(unittest.TestCase):
  def setUp(self):
    ClearBinaryManagerInstance(self)

  def tearDown(self):
    RestoreBinaryManagerInstance(self)

  def testSuccessfulInit(self):
    self.assertTrue(
        local_first_binary_manager.LocalFirstBinaryManager.NeedsInit())
    local_first_binary_manager.LocalFirstBinaryManager.Init(
        'build_dir', None, 'os_name', 'arch', ['ignored_dep'],
        'os_version')
    self.assertFalse(
        local_first_binary_manager.LocalFirstBinaryManager.NeedsInit())
    bm = local_first_binary_manager.GetInstance()
    self.assertEqual(bm._build_dir, 'build_dir')
    self.assertEqual(bm._os, 'os_name')
    self.assertEqual(bm._arch, 'arch')
    self.assertEqual(bm._os_version, 'os_version')
    self.assertEqual(bm._ignored_dependencies, ['ignored_dep'])

  def testAlreadyInitialized(self):
    local_first_binary_manager.LocalFirstBinaryManager.Init(
        None, None, None, None, None, None)
    with self.assertRaises(exceptions.InitializationError):
      local_first_binary_manager.LocalFirstBinaryManager.Init(
          None, None, None, None, None, None)

  def testGetInstanceUninitialized(self):
    with self.assertRaises(exceptions.InitializationError):
      local_first_binary_manager.GetInstance()
