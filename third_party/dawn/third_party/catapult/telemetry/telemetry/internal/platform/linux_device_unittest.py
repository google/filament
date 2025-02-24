# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import unittest
from unittest import mock

from telemetry.core import platform
from telemetry.internal.platform import linux_device


class LinuxDeviceTest(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self.host_platform = mock.create_autospec(platform.Platform, instance=True)

  def _set_os(self, os_name):
    self.host_platform.GetOSName.return_value = os_name

  def test_device_is_running_on_linux_if_found(self):
    with mock.patch.object(platform, 'GetHostPlatform') as m_plat:
      m_plat.return_value = self.host_platform
      self._set_os('linux')

      self.assertTrue(linux_device.IsRunningOnLinux())

  def test_device_is_running_on_linux_if_not_found(self):
    with mock.patch.object(platform, 'GetHostPlatform',
                           return_value=self.host_platform):
      self._set_os('foo_os')

      self.assertFalse(linux_device.IsRunningOnLinux())
