# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import argparse
import unittest
from unittest import mock

from telemetry.core import platform
from telemetry.internal.platform import cros_device
from telemetry.util import cmd_util


class CrOSDeviceTest(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self.host_platform = mock.create_autospec(platform.Platform, instance=True)

  def _set_os(self, os_name):
    self.host_platform.GetOSName.return_value = os_name

  def test_device_inits_as_localhost(self):
    device = cros_device.CrOSDevice(
        host_name=None,
        ssh_port=None,
        ssh_identity=None,
        is_local=None
    )
    self.assertIsNone(device.host_name)
    self.assertIsNone(device.ssh_port)
    self.assertIsNone(device.ssh_identity)

    self.assertEqual('ChromeOs with host localhost', device.name)
    self.assertEqual('cros:localhost', device.guid)

  def test_device_inits_with_host_name_reports_it(self):
    device = cros_device.CrOSDevice(
        host_name='127.0.0.1',
        ssh_port=None,
        ssh_identity=None,
        is_local=None
    )
    self.assertEqual('ChromeOs with host 127.0.0.1', device.name)
    self.assertEqual('cros:127.0.0.1', device.guid)

  def test_device_is_running_on_cros_if_found(self):
    with mock.patch.object(platform, 'GetHostPlatform') as m_plat:
      m_plat.return_value = self.host_platform
      self._set_os('chromeos')

      self.assertTrue(cros_device.IsRunningOnCrOS())
      self.assertTrue(cros_device.CrOSDevice.PlatformIsRunningOS())

  def test_device_is_running_on_cros_if_not_found(self):
    with mock.patch.object(platform, 'GetHostPlatform',
                           return_value=self.host_platform):
      self._set_os('linux')

      self.assertFalse(cros_device.IsRunningOnCrOS())
      self.assertFalse(cros_device.CrOSDevice.PlatformIsRunningOS())

  def test_find_available_devices_returns_devices_if_found(self):
    options = argparse.Namespace(remote='some_address',
                                 fetch_cros_remote=False,
                                 remote_ssh_port=50,
                                 ssh_identity='path/to/id_rsa')
    with mock.patch.object(platform, 'GetHostPlatform',
                           return_value=self.host_platform):
      self._set_os('chromeos')
      with mock.patch.object(cmd_util, 'HasSSH', return_value=True):
        devices = cros_device.CrOSDevice.FindAllAvailableDevices(options)

        self.assertEqual(len(devices), 1)
        self.assertIsInstance(devices[0], cros_device.CrOSDevice)
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
      self._set_os('chromeos')
      with mock.patch.object(cmd_util, 'HasSSH', return_value=False):
        devices = cros_device.CrOSDevice.FindAllAvailableDevices(options)

        self.assertEqual(len(devices), 1)
        self.assertIsInstance(devices[0], cros_device.CrOSDevice)
        self.assertEqual(devices[0].host_name, 'some_address')
        self.assertEqual(devices[0].is_local, True)

  def test_find_available_devices_returns_devices_if_no_ssh_and_no_chromeos(self):
    options = argparse.Namespace(remote='some_address',
                                 fetch_cros_remote=False,
                                 remote_ssh_port=None,
                                 ssh_identity=None)
    with mock.patch.object(platform, 'GetHostPlatform',
                           return_value=self.host_platform):
      self._set_os('linux')
      with mock.patch.object(cmd_util, 'HasSSH', return_value=False):
        devices = cros_device.CrOSDevice.FindAllAvailableDevices(options)

        self.assertFalse(devices)
