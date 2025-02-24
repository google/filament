# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock

from telemetry import decorators
from telemetry.internal.browser import browser_options
from telemetry.internal.platform import android_device
from telemetry.internal.platform import remote_platform_options
from telemetry.testing import system_stub

from devil.android import device_utils
from devil.android import device_denylist


class _BaseAndroidDeviceTest(unittest.TestCase):
  def setUp(self):
    def check_denylist_arg(denylist):
      self.assertTrue(denylist is None
                      or isinstance(denylist, device_denylist.Denylist))
      return mock.DEFAULT

    self._healthy_device_patcher = mock.patch(
        'devil.android.device_utils.DeviceUtils.HealthyDevices')
    self._healthy_device_mock = self._healthy_device_patcher.start()
    self._healthy_device_mock.side_effect = check_denylist_arg
    self._android_device_stub = system_stub.Override(
        android_device, ['subprocess'])

  def _GetMockDeviceUtils(self, device_serial):
    device = device_utils.DeviceUtils(device_serial)
    return device

  def tearDown(self):
    self._healthy_device_patcher.stop()
    self._android_device_stub.Restore()


class AndroidDeviceTest(_BaseAndroidDeviceTest):
  @decorators.Enabled('android')
  def testGetAllAttachedAndroidDevices(self):
    self._healthy_device_mock.return_value = [
        self._GetMockDeviceUtils('01'),
        self._GetMockDeviceUtils('02')]
    self.assertEqual(
        {'01', '02'},
        set(device.device_id for device in
            android_device.AndroidDevice.GetAllConnectedDevices(None)))

  @decorators.Enabled('android')
  @mock.patch('telemetry.internal.platform.android_device.logging.warning')
  def testNoAdbReturnsNone(self, warning_mock):
    finder_options = browser_options.BrowserFinderOptions()
    with (
        mock.patch('os.path.isabs', return_value=True)), (
            mock.patch('os.path.exists', return_value=False)):
      self.assertEqual(warning_mock.call_count, 0)
      self.assertIsNone(android_device.GetDevice(finder_options))

  # https://github.com/catapult-project/catapult/issues/3099 (Android)
  @decorators.Disabled('all')
  @mock.patch('telemetry.internal.platform.android_device.logging.warning')
  def testAdbNoDevicesReturnsNone(self, warning_mock):
    finder_options = browser_options.BrowserFinderOptions()
    with mock.patch('os.path.isabs', return_value=False):
      self._healthy_device_mock.return_value = []
      self.assertEqual(warning_mock.call_count, 0)
      self.assertIsNone(android_device.GetDevice(finder_options))

  # https://github.com/catapult-project/catapult/issues/3099 (Android)
  @decorators.Disabled('all')
  @mock.patch('telemetry.internal.platform.android_device.logging.warning')
  def testAdbTwoDevicesReturnsNone(self, warning_mock):
    finder_options = browser_options.BrowserFinderOptions()
    with mock.patch('os.path.isabs', return_value=False):
      self._healthy_device_mock.return_value = [
          self._GetMockDeviceUtils('015d14fec128220c'),
          self._GetMockDeviceUtils('015d14fec128220d')]
      device = android_device.GetDevice(finder_options)
      warning_mock.assert_called_with(
          'Multiple devices attached. Please specify one of the following:\n'
          '  --device=015d14fec128220c\n'
          '  --device=015d14fec128220d')
      self.assertIsNone(device)

  @decorators.Enabled('android')
  @mock.patch('telemetry.internal.platform.android_device.logging.warning')
  def testAdbPickOneDeviceReturnsDeviceInstance(self, warning_mock):
    finder_options = browser_options.BrowserFinderOptions()
    platform_options = remote_platform_options.AndroidPlatformOptions(
        device='555d14fecddddddd')  # pick one
    finder_options.remote_platform_options = platform_options
    with mock.patch('os.path.isabs', return_value=False):
      self._healthy_device_mock.return_value = [
          self._GetMockDeviceUtils('015d14fec128220c'),
          self._GetMockDeviceUtils('555d14fecddddddd')]
      device = android_device.GetDevice(finder_options)
      self.assertEqual(warning_mock.call_count, 0)
      self.assertEqual('555d14fecddddddd', device.device_id)

  # https://github.com/catapult-project/catapult/issues/3099 (Android)
  @decorators.Disabled('all')
  @mock.patch('telemetry.internal.platform.android_device.logging.warning')
  def testAdbOneDeviceReturnsDeviceInstance(self, warning_mock):
    finder_options = browser_options.BrowserFinderOptions()
    with mock.patch('os.path.isabs', return_value=False):
      self._healthy_device_mock.return_value = [
          self._GetMockDeviceUtils('015d14fec128220c')]
      device = android_device.GetDevice(finder_options)
      self.assertEqual(warning_mock.call_count, 0)
      self.assertEqual('015d14fec128220c', device.device_id)


class FindAllAvailableDevicesTest(_BaseAndroidDeviceTest):

  @decorators.Disabled('all')
  @mock.patch('telemetry.internal.platform.android_device.logging.warning')
  # https://github.com/catapult-project/catapult/issues/3099 (Android)
  def testAdbNoDeviceReturnsEmptyList(self, warning_mock):
    finder_options = browser_options.BrowserFinderOptions()
    with mock.patch('os.path.isabs', return_value=False):
      self._healthy_device_mock.return_value = []
      devices = android_device.FindAllAvailableDevices(finder_options)
      self.assertEqual(warning_mock.call_count, 0)
      self.assertIsNotNone(devices)
      self.assertEqual(len(devices), 0)


  @decorators.Disabled('all')
  @mock.patch('telemetry.internal.platform.android_device.logging.warning')
  # https://github.com/catapult-project/catapult/issues/3099 (Android)
  def testAdbOneDeviceReturnsListWithOneDeviceInstance(self, warning_mock):
    finder_options = browser_options.BrowserFinderOptions()
    with mock.patch('os.path.isabs', return_value=False):
      self._healthy_device_mock.return_value = [
          self._GetMockDeviceUtils('015d14fec128220c')]
      devices = android_device.FindAllAvailableDevices(finder_options)
      self.assertEqual(warning_mock.call_count, 0)
      self.assertIsNotNone(devices)
      self.assertEqual(len(devices), 1)
      self.assertEqual('015d14fec128220c', devices[0].device_id)


  @decorators.Disabled('all')
  @mock.patch('telemetry.internal.platform.android_device.logging.warning')
  # https://github.com/catapult-project/catapult/issues/3099 (Android)
  def testAdbMultipleDevicesReturnsListWithAllDeviceInstances(self, warning_mock):
    finder_options = browser_options.BrowserFinderOptions()
    with mock.patch('os.path.isabs', return_value=False):
      self._healthy_device_mock.return_value = [
          self._GetMockDeviceUtils('015d14fec128220c'),
          self._GetMockDeviceUtils('015d14fec128220d'),
          self._GetMockDeviceUtils('015d14fec128220e')]
      devices = android_device.FindAllAvailableDevices(finder_options)
      self.assertEqual(warning_mock.call_count, 0)
      self.assertIsNotNone(devices)
      self.assertEqual(len(devices), 3)
      self.assertEqual(devices[0].guid, '015d14fec128220c')
      self.assertEqual(devices[1].guid, '015d14fec128220d')
      self.assertEqual(devices[2].guid, '015d14fec128220e')
