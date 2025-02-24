# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import argparse
import unittest
from unittest import mock

from telemetry.core import platform
from telemetry.internal.platform import linux_based_device
from telemetry.util import cmd_util


class LinuxBasedDeviceTest(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self.host_platform = mock.create_autospec(platform.Platform, instance=True)

  def _set_os(self, os_name):
    self.host_platform.GetOSName.return_value = os_name

  def test_device_is_running_on_linux_if_found(self):
    with mock.patch.object(platform, 'GetHostPlatform') as m_plat:
      m_plat.return_value = self.host_platform
      self._set_os('linux')

      self.assertTrue(linux_based_device.LinuxBasedDevice.PlatformIsRunningOS())

  def test_device_is_running_on_linux_if_not_found(self):
    with mock.patch.object(platform, 'GetHostPlatform',
                           return_value=self.host_platform):
      self._set_os('foo_os')

      self.assertFalse(
          linux_based_device.LinuxBasedDevice.PlatformIsRunningOS())

  def test_find_available_devices_returns_devices_if_found(self):
    options = argparse.Namespace(remote='some_address',
                                 fetch_cros_remote=False,
                                 remote_ssh_port=50,
                                 ssh_identity='path/to/id_rsa')
    with mock.patch.object(platform, 'GetHostPlatform',
                           return_value=self.host_platform):
      self._set_os('linux')
      with mock.patch.object(cmd_util, 'HasSSH', return_value=True):
        devices = linux_based_device.LinuxBasedDevice.FindAllAvailableDevices(
            options)

        self.assertEqual(len(devices), 1)
        self.assertIsInstance(devices[0], linux_based_device.LinuxBasedDevice)
        self.assertEqual(devices[0].host_name, 'some_address')
        self.assertEqual(devices[0].ssh_port, 50)
        self.assertEqual(devices[0].ssh_identity, 'path/to/id_rsa')
        self.assertEqual(devices[0].is_local, False)

  def test_find_available_devices_returns_devices_if_no_ssh(self):
    options = argparse.Namespace(remote='some_address',
                                 fetch_cros_remote=False,
                                 remote_ssh_port=None,
                                 ssh_identity=None)
    with mock.patch.object(platform, 'GetHostPlatform',
                           return_value=self.host_platform):
      self._set_os('linux')
      with mock.patch.object(cmd_util, 'HasSSH', return_value=False):
        devices = linux_based_device.LinuxBasedDevice.FindAllAvailableDevices(
            options)

        self.assertEqual(len(devices), 1)
        self.assertIsInstance(devices[0], linux_based_device.LinuxBasedDevice)
        self.assertEqual(devices[0].host_name, 'some_address')
        self.assertEqual(devices[0].is_local, True)

  def test_find_available_devices_returns_devices_if_no_ssh_and_no_chromeos(
      self):
    options = argparse.Namespace(remote='some_address',
                                 remote_ssh_port=None,
                                 ssh_identity=None)
    with mock.patch.object(platform, 'GetHostPlatform',
                           return_value=self.host_platform):
      self._set_os('foo_os')
      with mock.patch.object(cmd_util, 'HasSSH', return_value=False):
        devices = linux_based_device.LinuxBasedDevice.FindAllAvailableDevices(
            options)

        self.assertFalse(devices)
