#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import os
import posixpath
import shutil
import sys
import tempfile
import unittest

if __name__ == '__main__':
  sys.path.append(
      os.path.abspath(
          os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from devil import base_error
from devil import devil_env
from devil.android import device_temp_file
from devil.android import device_test_case
from devil.android import device_utils
from devil.android.tools import system_app

logger = logging.getLogger(__name__)


class SystemAppDeviceTest(device_test_case.DeviceTestCase):

  PACKAGE = 'com.google.android.webview'

  def setUp(self):
    super(SystemAppDeviceTest, self).setUp()
    self._device = device_utils.DeviceUtils(self.serial)
    self._original_paths = self._device.GetApplicationPaths(self.PACKAGE)
    self._apk_cache_dir = tempfile.mkdtemp()
    # Host location -> device location
    self._cached_apks = {}
    for o in self._original_paths:
      h = os.path.join(self._apk_cache_dir, posixpath.basename(o))
      self._device.PullFile(o, h, timeout=60)
      self._cached_apks[h] = o

  def tearDown(self):
    final_paths = self._device.GetApplicationPaths(self.PACKAGE)
    if self._original_paths != final_paths:
      try:
        self._device.Uninstall(self.PACKAGE)
      except Exception:  # pylint: disable=broad-except
        pass

      with system_app.EnableSystemAppModification(self._device):
        for cached_apk, install_path in self._cached_apks.items():
          try:
            with device_temp_file.DeviceTempFile(self._device.adb) as tmp:
              self._device.adb.Push(cached_apk, tmp.name)
              self._device.RunShellCommand(['mv', tmp.name, install_path],
                                           as_root=True,
                                           check_return=True)
          except base_error.BaseError:
            logger.warning('Failed to reinstall %s',
                           os.path.basename(cached_apk))

    try:
      shutil.rmtree(self._apk_cache_dir)
    except IOError:
      logger.error('Unable to remove app cache directory.')

    super(SystemAppDeviceTest, self).tearDown()

  def _check_preconditions(self):
    if not self._original_paths:
      self.skipTest(
          '%s is not installed on %s' % (self.PACKAGE, str(self._device)))
    if not any(p.startswith('/system/') for p in self._original_paths):
      self.skipTest('%s is not installed in a system location on %s' %
                    (self.PACKAGE, str(self._device)))

  def testReplace(self):
    self._check_preconditions()
    replacement = devil_env.config.FetchPath(
        'empty_system_webview', device=self._device)
    with system_app.ReplaceSystemApp(self._device, replacement):
      replaced_paths = self._device.GetApplicationPaths(self.PACKAGE)
      self.assertNotEqual(self._original_paths, replaced_paths)
    restored_paths = self._device.GetApplicationPaths(self.PACKAGE)
    self.assertEqual(self._original_paths, restored_paths)

  def testRemove(self):
    self._check_preconditions()
    system_app.RemoveSystemApps(self._device, [self.PACKAGE])
    removed_paths = self._device.GetApplicationPaths(self.PACKAGE)
    self.assertEqual([], removed_paths)

  def testInstallPrivileged(self):
    self._check_preconditions()
    privileged_path = devil_env.config.FetchPath('empty_system_webview',
                                                 device=self._device)
    system_app.InstallPrivilegedApps(self._device,
                                     [(privileged_path, '/system')])
    installed_paths = self._device.GetApplicationPaths(self.PACKAGE)
    self.assertEqual(len(installed_paths), 1)
    self.assertIn('/system/priv-app', installed_paths[0])


if __name__ == '__main__':
  unittest.main()
