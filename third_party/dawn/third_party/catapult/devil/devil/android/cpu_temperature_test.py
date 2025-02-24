#!/usr/bin/env python
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Unit tests for the contents of cpu_temperature.py
"""

# pylint: disable=unused-argument

import logging
import unittest

from unittest import mock

from devil.android import cpu_temperature
from devil.android import device_utils
from devil.utils import mock_calls
from devil.android.sdk import adb_wrapper


class CpuTemperatureTest(mock_calls.TestCase):
  def setUp(self):
    # Mock the device
    self.mock_device = mock.Mock(spec=device_utils.DeviceUtils)
    self.mock_device.build_product = 'blueline'
    self.mock_device.adb = mock.Mock(spec=adb_wrapper.AdbWrapper)
    self.mock_device.FileExists.return_value = True

    with mock.patch('devil.android.perf.perf_control.PerfControl'):
      self.cpu_temp = cpu_temperature.CpuTemperature(self.mock_device)
      self.cpu_temp.InitThermalDeviceInformation()


class CpuTemperatureInitTest(unittest.TestCase):
  @mock.patch('devil.android.perf.perf_control.PerfControl', mock.Mock())
  def testInitWithDeviceUtil(self):
    d = mock.Mock(spec=device_utils.DeviceUtils)
    d.build_product = 'blueline'
    c = cpu_temperature.CpuTemperature(d)
    self.assertEqual(d, c.GetDeviceForTesting())

  def testInitWithMissing_fails(self):
    with self.assertRaises(TypeError):
      cpu_temperature.CpuTemperature(None)
    with self.assertRaises(TypeError):
      cpu_temperature.CpuTemperature('')


class CpuTemperatureGetThermalDeviceInformationTest(CpuTemperatureTest):
  @mock.patch('devil.android.perf.perf_control.PerfControl', mock.Mock())
  def testGetThermalDeviceInformation_noneWhenIncorrectLabel(self):
    invalid_device = mock.Mock(spec=device_utils.DeviceUtils)
    invalid_device.build_product = 'invalid_name'
    c = cpu_temperature.CpuTemperature(invalid_device)
    c.InitThermalDeviceInformation()
    self.assertEqual(c.GetDeviceInfoForTesting(), None)

  def testGetThermalDeviceInformation_getsCorrectInformation(self):
    correct_information = {
        'cpu0': '/sys/class/thermal/thermal_zone11/temp',
        'cpu1': '/sys/class/thermal/thermal_zone12/temp',
        'cpu2': '/sys/class/thermal/thermal_zone13/temp',
        'cpu3': '/sys/class/thermal/thermal_zone14/temp',
        'cpu4': '/sys/class/thermal/thermal_zone15/temp',
        'cpu5': '/sys/class/thermal/thermal_zone16/temp',
        'cpu6': '/sys/class/thermal/thermal_zone17/temp',
        'cpu7': '/sys/class/thermal/thermal_zone18/temp'
    }

    self.assertDictEqual(
        correct_information,
        self.cpu_temp.GetDeviceInfoForTesting().get('cpu_temps')
    )


class CpuTemperatureIsSupportedTest(CpuTemperatureTest):
  @mock.patch('devil.android.perf.perf_control.PerfControl', mock.Mock())
  def testIsSupported_returnsTrue(self):
    d = mock.Mock(spec=device_utils.DeviceUtils)
    d.build_product = 'blueline'
    d.FileExists.return_value = True
    c = cpu_temperature.CpuTemperature(d)
    self.assertTrue(c.IsSupported())

  @mock.patch('devil.android.perf.perf_control.PerfControl', mock.Mock())
  def testIsSupported_returnsFalse(self):
    d = mock.Mock(spec=device_utils.DeviceUtils)
    d.build_product = 'blueline'
    d.FileExists.return_value = False
    c = cpu_temperature.CpuTemperature(d)
    self.assertFalse(c.IsSupported())


class CpuTemperatureLetCpuCoolToTemperatureTest(CpuTemperatureTest):
  # Return values for the mock side effect
  cooling_down0 = (
      [45000
       for _ in range(8)] + [43000
                             for _ in range(8)] + [41000 for _ in range(8)])

  @mock.patch('time.sleep', mock.Mock())
  def testLetBatteryCoolToTemperature_coolWithin24Calls(self):
    self.mock_device.ReadFile = mock.Mock(side_effect=self.cooling_down0)
    self.cpu_temp.LetCpuCoolToTemperature(42)
    self.mock_device.ReadFile.assert_called()
    self.assertEqual(self.mock_device.ReadFile.call_count, 24)

  cooling_down1 = [45000 for _ in range(8)] + [41000 for _ in range(16)]

  @mock.patch('time.sleep', mock.Mock())
  def testLetBatteryCoolToTemperature_coolWithin16Calls(self):
    self.mock_device.ReadFile = mock.Mock(side_effect=self.cooling_down1)
    self.cpu_temp.LetCpuCoolToTemperature(42)
    self.mock_device.ReadFile.assert_called()
    self.assertEqual(self.mock_device.ReadFile.call_count, 16)

  constant_temp = [45000 for _ in range(40)]

  @mock.patch('time.sleep', mock.Mock())
  def testLetBatteryCoolToTemperature_timeoutAfterThree(self):
    self.mock_device.ReadFile = mock.Mock(side_effect=self.constant_temp)
    self.cpu_temp.LetCpuCoolToTemperature(42)
    self.mock_device.ReadFile.assert_called()
    self.assertEqual(self.mock_device.ReadFile.call_count, 24)


if __name__ == '__main__':
  logging.getLogger().setLevel(logging.DEBUG)
  unittest.main(verbosity=2)
