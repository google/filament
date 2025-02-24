# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from unittest import mock

from devil.android import device_utils
from devil.android.perf import perf_control
from devil.android.sdk import adb_wrapper


def _ShellCommandHandler(  # pylint: disable=unused-argument
    cmd,
    shell=False,
    check_return=False,
    cwd=None,
    env=None,
    run_as=None,
    as_root=False,
    single_line=False,
    large_output=False,
    raw_output=False,
    timeout=None,
    retries=None):
  if cmd.startswith('for CPU in '):
    if 'scaling_available_governors' in cmd:
      contents = 'interactive ondemand userspace powersave performance'
      return [contents + '\n%~%0%~%'] * 4
    if 'cat "$CPU/online"' in cmd:
      return ['1\n%~%0%~%'] * 4
  assert False, 'Should not be called with cmd: {}'.format(cmd)
  # This is unreachable
  # we are just adding this return to quiet pylint
  return None


class PerfControlTest(unittest.TestCase):
  @staticmethod
  def _MockOutLowLevelPerfControlMethods(perf_control_object):
    # pylint: disable=protected-access
    perf_control_object.SetScalingGovernor = mock.Mock()
    perf_control_object._ForceAllCpusOnline = mock.Mock()
    perf_control_object._SetScalingMaxFreqForCpus = mock.Mock()
    perf_control_object._SetMaxGpuClock = mock.Mock()

  # pylint: disable=no-self-use
  def testNexus5HighPerfMode(self):
    # Mock out the device state for PerfControl.
    cpu_list = ['cpu%d' % cpu for cpu in range(4)]
    mock_device = mock.Mock(spec=device_utils.DeviceUtils)
    mock_device.product_model = 'Nexus 5'
    mock_device.adb = mock.Mock(spec=adb_wrapper.AdbWrapper)
    mock_device.ListDirectory.return_value = cpu_list + ['cpufreq']
    mock_device.FileExists.return_value = True
    mock_device.RunShellCommand = mock.Mock(side_effect=_ShellCommandHandler)
    pc = perf_control.PerfControl(mock_device)
    self._MockOutLowLevelPerfControlMethods(pc)

    # Verify.
    # pylint: disable=protected-access
    # pylint: disable=no-member
    pc.SetHighPerfMode()
    mock_device.EnableRoot.assert_called_once_with()
    pc._ForceAllCpusOnline.assert_called_once_with(True)
    pc.SetScalingGovernor.assert_called_once_with('performance')
    pc._SetScalingMaxFreqForCpus.assert_called_once_with(
        1190400, ' '.join(cpu_list))
    pc._SetMaxGpuClock.assert_called_once_with(200000000)

  def testNexus5XHighPerfMode(self):
    # Mock out the device state for PerfControl.
    cpu_list = ['cpu%d' % cpu for cpu in range(6)]
    mock_device = mock.Mock(spec=device_utils.DeviceUtils)
    mock_device.product_model = 'Nexus 5X'
    mock_device.adb = mock.Mock(spec=adb_wrapper.AdbWrapper)
    mock_device.ListDirectory.return_value = cpu_list + ['cpufreq']
    mock_device.FileExists.return_value = True
    mock_device.RunShellCommand = mock.Mock(side_effect=_ShellCommandHandler)
    pc = perf_control.PerfControl(mock_device)
    self._MockOutLowLevelPerfControlMethods(pc)

    # Verify.
    # pylint: disable=protected-access
    # pylint: disable=no-member
    pc.SetHighPerfMode()
    mock_device.EnableRoot.assert_called_once_with()
    pc._ForceAllCpusOnline.assert_called_once_with(True)
    pc.SetScalingGovernor.assert_called_once_with('performance')
    pc._SetScalingMaxFreqForCpus.assert_called_once_with(
        1248000, ' '.join(cpu_list))
    pc._SetMaxGpuClock.assert_called_once_with(300000000)

  def testNexus5XDefaultPerfMode(self):
    # Mock out the device state for PerfControl.
    cpu_list = ['cpu%d' % cpu for cpu in range(6)]
    mock_device = mock.Mock(spec=device_utils.DeviceUtils)
    mock_device.product_model = 'Nexus 5X'
    mock_device.adb = mock.Mock(spec=adb_wrapper.AdbWrapper)
    mock_device.ListDirectory.return_value = cpu_list + ['cpufreq']
    mock_device.FileExists.return_value = True
    mock_device.RunShellCommand = mock.Mock(side_effect=_ShellCommandHandler)
    pc = perf_control.PerfControl(mock_device)
    self._MockOutLowLevelPerfControlMethods(pc)

    # Verify.
    # pylint: disable=protected-access
    # pylint: disable=no-member
    pc.SetDefaultPerfMode()
    pc.SetScalingGovernor.assert_called_once_with('ondemand')
    pc._SetScalingMaxFreqForCpus.assert_any_call(1440000, 'cpu0 cpu1 cpu2 cpu3')
    pc._SetScalingMaxFreqForCpus.assert_any_call(1824000, 'cpu4 cpu5')
    pc._SetMaxGpuClock.assert_called_once_with(600000000)
    pc._ForceAllCpusOnline.assert_called_once_with(False)
