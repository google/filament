# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock

from telemetry.core import exceptions
from telemetry.internal.util import binary_manager


class BinaryManagerTest(unittest.TestCase):
  def setUp(self):
    # We need to preserve the real initialized dependecny_manager.
    self.actual_binary_manager = binary_manager._binary_manager
    binary_manager._binary_manager = None

  def tearDown(self):
    binary_manager._binary_manager = self.actual_binary_manager

  def testReinitialization(self):
    binary_manager.InitDependencyManager(None)
    self.assertRaises(exceptions.InitializationError,
                      binary_manager.InitDependencyManager, None)

  @mock.patch('py_utils.binary_manager.BinaryManager')
  def testFetchPathInitialized(self, py_utils_binary_manager_mock):
    expected = [
        mock.call([
            'base_config_object', binary_manager.TELEMETRY_PROJECT_CONFIG,
            binary_manager.CHROME_BINARY_CONFIG
        ]),
        mock.call().FetchPath('dep', 'plat', 'arch', None),
        mock.call().FetchPath('dep', 'plat', 'arch', 'version'),
    ]
    binary_manager.InitDependencyManager(['base_config_object'])
    binary_manager.FetchPath('dep', 'plat', 'arch')
    binary_manager.FetchPath('dep', 'plat', 'arch', 'version')
    py_utils_binary_manager_mock.assert_has_calls(expected)

  def testFetchPathUninitialized(self):
    self.assertRaises(exceptions.InitializationError,
                      binary_manager.FetchPath, 'dep', 'plat', 'arch')

  @mock.patch('py_utils.binary_manager.BinaryManager')
  def testLocalPathInitialized(self, py_utils_binary_manager_mock):
    expected = [
        mock.call([
            'base_config_object', binary_manager.TELEMETRY_PROJECT_CONFIG,
            binary_manager.CHROME_BINARY_CONFIG
        ]),
        mock.call().LocalPath('dep', 'plat', 'arch', None),
        mock.call().LocalPath('dep', 'plat', 'arch', 'version'),
    ]
    binary_manager.InitDependencyManager(['base_config_object'])
    binary_manager.LocalPath('dep', 'plat', 'arch')
    binary_manager.LocalPath('dep', 'plat', 'arch', 'version')
    py_utils_binary_manager_mock.assert_has_calls(expected)

  def testLocalPathUninitialized(self):
    self.assertRaises(exceptions.InitializationError,
                      binary_manager.LocalPath, 'dep', 'plat', 'arch')
