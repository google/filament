# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=W0212

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))

from devil.android import device_test_case
from devil.android import device_utils
from devil.android.perf import perf_control


class TestPerfControl(device_test_case.DeviceTestCase):
  def setUp(self):
    super(TestPerfControl, self).setUp()
    if not os.getenv('BUILDTYPE'):
      os.environ['BUILDTYPE'] = 'Debug'
    self._device = device_utils.DeviceUtils(self.serial)

  def testHighPerfMode(self):
    perf = perf_control.PerfControl(self._device)
    try:
      perf.SetPerfProfilingMode()
      cpu_info = perf.GetCpuInfo()
      self.assertEqual(len(perf._cpu_files), len(cpu_info))
      for _, online, governor in cpu_info:
        self.assertTrue(online)
        self.assertEqual('performance', governor)
    finally:
      perf.SetDefaultPerfMode()

  def testOverrideScalingGovernor(self):
    perf = perf_control.PerfControl(self._device)
    try:
      # Set all CPUs to the "performance" governor.
      perf.SetPerfProfilingMode()
      cpu_info = perf.GetCpuInfo()
      # Temporarily override all governors.
      with perf.OverrideScalingGovernor('powersave'):
        cpu_info = perf.GetCpuInfo()
        for _, _, new_governor in cpu_info:
          self.assertEqual(new_governor, 'powersave')
      # Check that original governors were restored.
      cpu_info = perf.GetCpuInfo()
      for _, _, governor in cpu_info:
        self.assertEqual(governor, 'performance')
    finally:
      perf.SetDefaultPerfMode()


if __name__ == '__main__':
  unittest.main()
