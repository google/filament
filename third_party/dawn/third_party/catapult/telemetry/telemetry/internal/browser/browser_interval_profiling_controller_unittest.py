# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock

from devil.android.sdk import version_codes

from telemetry.internal.browser import browser_interval_profiling_controller


class FakeDevice():
  def __init__(self, build_version_sdk):
    super().__init__()
    self.build_version_sdk = build_version_sdk


class FakeAndroidPlatformBackend():
  def __init__(self, build_version_sdk):
    super().__init__()
    self.device = FakeDevice(build_version_sdk)

  def GetOSName(self):
    return 'android'

class FakeChromeOSPlatformBackend():
  def GetOSName(self):
    return 'chromeos'

  def GetFileContents(self, _):
    return '0'

  def PushContents(self, *_):
    return

class FakeLinuxPlatformBackend():
  def GetOSName(self):
    return 'linux'


class FakeWindowsPlatformBackend():
  def GetOSName(self):
    return 'windows'


class FakePossibleBrowser():
  def __init__(self, platform_backend):
    self._platform_backend = platform_backend

  def AddExtraBrowserArg(self, _):
    pass


class BrowserIntervalProfilingControllerTest(unittest.TestCase):
  def testSupportedAndroidWithValidSamplePeriod(self):
    possible_browser = FakePossibleBrowser(
        FakeAndroidPlatformBackend(version_codes.OREO))
    profiling_mod = browser_interval_profiling_controller

    with mock.patch.multiple(
        'telemetry.internal.browser.browser_interval_profiling_controller',
        _AndroidController=mock.DEFAULT,
        _LinuxController=mock.DEFAULT,
        _ChromeOSController=mock.DEFAULT) as mock_classes:
      controller = profiling_mod.BrowserIntervalProfilingController(
          possible_browser, '', ['period1', 'period2'], 1, [])
      with controller.SamplePeriod('period1', None):
        pass
      with controller.SamplePeriod('period2', None):
        pass
      self.assertEqual(mock_classes['_AndroidController'].call_count, 1)
      self.assertEqual(mock_classes['_LinuxController'].call_count, 0)
      self.assertEqual(mock_classes['_ChromeOSController'].call_count, 0)
      self.assertTrue(controller._platform_controller)
      self.assertEqual(
          controller._platform_controller.SamplePeriod.call_count, 2)

  def testSupportedAndroidWithInvalidSamplePeriod(self):
    possible_browser = FakePossibleBrowser(
        FakeAndroidPlatformBackend(version_codes.OREO))
    profiling_mod = browser_interval_profiling_controller

    with mock.patch.multiple(
        'telemetry.internal.browser.browser_interval_profiling_controller',
        _AndroidController=mock.DEFAULT,
        _LinuxController=mock.DEFAULT,
        _ChromeOSController=mock.DEFAULT) as mock_classes:
      controller = profiling_mod.BrowserIntervalProfilingController(
          possible_browser, '', [], 1, '')
      with controller.SamplePeriod('period1', None):
        pass
      self.assertEqual(mock_classes['_AndroidController'].call_count, 0)
      self.assertEqual(mock_classes['_LinuxController'].call_count, 0)
      self.assertEqual(mock_classes['_ChromeOSController'].call_count, 0)
      self.assertIs(controller._platform_controller, None)

  def testSupportedAndroidWithSystemWideProfiling(self):
    possible_browser = FakePossibleBrowser(
        FakeAndroidPlatformBackend(version_codes.OREO))
    profiling_mod = browser_interval_profiling_controller

    with self.assertRaises(ValueError) as context:
      profiling_mod.BrowserIntervalProfilingController(
          possible_browser, 'system_wide', ['period1'], 1, [])
    self.assertTrue('System-wide profiling is not supported on Android.'
                    in repr(context.exception))

  def testSupportedAndroidWithProfilerOptions(self):
    possible_browser = FakePossibleBrowser(
        FakeAndroidPlatformBackend(version_codes.OREO))
    profiling_mod = browser_interval_profiling_controller

    with self.assertRaises(ValueError) as context:
      profiling_mod.BrowserIntervalProfilingController(
          possible_browser, '', ['period1'], 1, ['some profiler options'])
    self.assertTrue(
        'Additional arguments to the profiler is not supported on Android.'
        in repr(context.exception))

  def testUnsupportedAndroidWithValidSamplePeriod(self):
    possible_browser = FakePossibleBrowser(
        FakeAndroidPlatformBackend(version_codes.KITKAT))
    profiling_mod = browser_interval_profiling_controller

    with mock.patch.multiple(
        'telemetry.internal.browser.browser_interval_profiling_controller',
        _AndroidController=mock.DEFAULT,
        _LinuxController=mock.DEFAULT,
        _ChromeOSController=mock.DEFAULT) as mock_classes:
      controller = profiling_mod.BrowserIntervalProfilingController(
          possible_browser, '', ['period1'], 1, [])
      with controller.SamplePeriod('period1', None):
        pass
      self.assertEqual(mock_classes['_AndroidController'].call_count, 0)
      self.assertEqual(mock_classes['_LinuxController'].call_count, 0)
      self.assertEqual(mock_classes['_ChromeOSController'].call_count, 0)
      self.assertIs(controller._platform_controller, None)

  def testUnsupportedAndroidWithInvalidSamplePeriod(self):
    possible_browser = FakePossibleBrowser(
        FakeAndroidPlatformBackend(version_codes.KITKAT))
    profiling_mod = browser_interval_profiling_controller

    with mock.patch.multiple(
        'telemetry.internal.browser.browser_interval_profiling_controller',
        _AndroidController=mock.DEFAULT,
        _LinuxController=mock.DEFAULT,
        _ChromeOSController=mock.DEFAULT) as mock_classes:
      controller = profiling_mod.BrowserIntervalProfilingController(
          possible_browser, '', [], 1, [])
      with controller.SamplePeriod('period1', None):
        pass
      self.assertEqual(mock_classes['_AndroidController'].call_count, 0)
      self.assertEqual(mock_classes['_LinuxController'].call_count, 0)
      self.assertEqual(mock_classes['_ChromeOSController'].call_count, 0)
      self.assertIs(controller._platform_controller, None)

  def testLinuxWithValidAndInvalidSamplePeriods(self):
    possible_browser = FakePossibleBrowser(FakeLinuxPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with mock.patch.multiple(
        'telemetry.internal.browser.browser_interval_profiling_controller',
        _AndroidController=mock.DEFAULT,
        _LinuxController=mock.DEFAULT,
        _ChromeOSController=mock.DEFAULT) as mock_classes:
      controller = profiling_mod.BrowserIntervalProfilingController(
          possible_browser, '', ['period1', 'period6'], 1, [])
      with controller.SamplePeriod('period1', None):
        pass
      with controller.SamplePeriod('period2', None):
        pass
      self.assertEqual(mock_classes['_AndroidController'].call_count, 0)
      self.assertEqual(mock_classes['_LinuxController'].call_count, 1)
      self.assertEqual(mock_classes['_ChromeOSController'].call_count, 0)
      self.assertTrue(controller._platform_controller)
      # Only one sample period, because 'period2' not in periods args to
      # constructor.
      self.assertEqual(
          controller._platform_controller.SamplePeriod.call_count, 1)

  def testLinuxWithInvalidSamplePeriod(self):
    possible_browser = FakePossibleBrowser(FakeLinuxPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with mock.patch.multiple(
        'telemetry.internal.browser.browser_interval_profiling_controller',
        _AndroidController=mock.DEFAULT,
        _LinuxController=mock.DEFAULT,
        _ChromeOSController=mock.DEFAULT) as mock_classes:
      controller = profiling_mod.BrowserIntervalProfilingController(
          possible_browser, '', [], 1, [])
      with controller.SamplePeriod('period1', None):
        pass
      self.assertEqual(mock_classes['_AndroidController'].call_count, 0)
      self.assertEqual(mock_classes['_LinuxController'].call_count, 0)
      self.assertEqual(mock_classes['_ChromeOSController'].call_count, 0)
      self.assertIs(controller._platform_controller, None)

  def testLinuxWithSystemWideProfiling(self):
    possible_browser = FakePossibleBrowser(FakeLinuxPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with self.assertRaises(ValueError) as context:
      profiling_mod.BrowserIntervalProfilingController(
          possible_browser, 'system_wide', ['period1'], 1, [])
    self.assertTrue('Only profiling renderer main thread is supported on Linux.'
                    ' Got process name \"system_wide\" and thread name \"\".'
                    in repr(context.exception))

  def testLinuxWithProfilerOptions(self):
    possible_browser = FakePossibleBrowser(FakeLinuxPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with self.assertRaises(ValueError) as context:
      profiling_mod.BrowserIntervalProfilingController(
          possible_browser, '', ['period1'], 1, ['some profiler options'])
    self.assertTrue(
        'Additional arguments to the profiler is not supported on Linux.'
        in repr(context.exception))

  def testChromeOSWithValidSamplePeriod(self):
    possible_browser = FakePossibleBrowser(FakeChromeOSPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with mock.patch.multiple(
        'telemetry.internal.browser.browser_interval_profiling_controller',
        _AndroidController=mock.DEFAULT,
        _LinuxController=mock.DEFAULT,
        _ChromeOSController=mock.DEFAULT) as mock_classes:
      controller = profiling_mod.BrowserIntervalProfilingController(
          possible_browser, 'system_wide', ['period1', 'period2'], 1,
          ['some profiler options'])
      with controller.SamplePeriod('period1', None):
        pass
      with controller.SamplePeriod('period2', None):
        pass
      self.assertEqual(mock_classes['_AndroidController'].call_count, 0)
      self.assertEqual(mock_classes['_LinuxController'].call_count, 0)
      self.assertEqual(mock_classes['_ChromeOSController'].call_count, 1)
      self.assertTrue(controller._platform_controller)
      self.assertEqual(
          controller._platform_controller.SamplePeriod.call_count, 2)

  def testChromeOSWithInvalidSamplePeriod(self):
    possible_browser = FakePossibleBrowser(FakeChromeOSPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with mock.patch.multiple(
        'telemetry.internal.browser.browser_interval_profiling_controller',
        _AndroidController=mock.DEFAULT,
        _LinuxController=mock.DEFAULT,
        _ChromeOSController=mock.DEFAULT) as mock_classes:
      controller = profiling_mod.BrowserIntervalProfilingController(
          possible_browser, 'system_wide', [], 1, ['some profiler options'])
      with controller.SamplePeriod('period1', None):
        pass
      self.assertEqual(mock_classes['_AndroidController'].call_count, 0)
      self.assertEqual(mock_classes['_LinuxController'].call_count, 0)
      self.assertEqual(mock_classes['_ChromeOSController'].call_count, 0)
      self.assertIs(controller._platform_controller, None)

  def testChromeOSWithThreadName(self):
    possible_browser = FakePossibleBrowser(FakeChromeOSPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with self.assertRaises(ValueError) as context:
      profiling_mod.BrowserIntervalProfilingController(
          possible_browser, 'system_wide:some_thread', ['period1'], 1,
          ['some profiler options'])
    self.assertTrue(
        'Thread name should be empty for system-wide profiling on ChromeOS.'
        ' Got thread name \"some_thread\".' in repr(context.exception))

  def testChromeOSWithOutSystemWideProfiling(self):
    possible_browser = FakePossibleBrowser(FakeChromeOSPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with self.assertRaises(ValueError) as context:
      profiling_mod.BrowserIntervalProfilingController(
          possible_browser, '', ['period1'], 1, ['some profiler options'])
    self.assertTrue(
        'Only system-wide profiling is supported on ChromeOS.'
        ' Got process name \"\".'
        in repr(context.exception))

  def testChromeOSWithOutProfilerOptions(self):
    possible_browser = FakePossibleBrowser(FakeChromeOSPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with self.assertRaises(ValueError) as context:
      profiling_mod.BrowserIntervalProfilingController(
          possible_browser, 'system_wide', ['period1'], 1, [])
    self.assertTrue('Profiler options must be provided to run the linux perf'
                    ' tool on ChromeOS.' in repr(context.exception))

  def testChromeOSWithProfilerOptionsContainsInvalidPerfCommand(self):
    possible_browser = FakePossibleBrowser(FakeChromeOSPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with self.assertRaises(ValueError) as context:
      profiling_mod.BrowserIntervalProfilingController(
          possible_browser, 'system_wide', ['period1'], 1,
          ['random', 'command'])
    self.assertTrue(
        'Only the record and stat perf subcommands are allowed.'
        ' Got \"random\" perf subcommand.'
        in repr(context.exception))

  def testChromeOSWithProfilerOptionsContainsOutputFlag(self):
    possible_browser = FakePossibleBrowser(FakeChromeOSPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with self.assertRaises(ValueError) as context:
      profiling_mod.BrowserIntervalProfilingController(
          possible_browser, 'system_wide', ['period1'], 1,
          ['record', '-o', 'file'])
    self.assertTrue(
        "Cannot pass the output filename flag in the profiler options."
        " Constructed command ['/usr/bin/perf', 'record', '-o', 'file',"
        " '-a']."
        in repr(context.exception))

  def testChromeOSWithProfilerOptionsContainsSubcommand(self):
    possible_browser = FakePossibleBrowser(FakeChromeOSPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with self.assertRaises(ValueError) as context:
      profiling_mod.BrowserIntervalProfilingController(
          possible_browser, 'system_wide', ['period1'], 1,
          ['record', '--', 'some subcommand'])
    self.assertTrue(
        "Cannot pass a command to run in the profiler options."
        " Constructed command ['/usr/bin/perf', 'record', '--',"
        " 'some subcommand', '-a']."
        in repr(context.exception))

  def testUnsupportedPlatformWithValidSamplePeriod(self):
    possible_browser = FakePossibleBrowser(FakeWindowsPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with mock.patch.multiple(
        'telemetry.internal.browser.browser_interval_profiling_controller',
        _AndroidController=mock.DEFAULT,
        _LinuxController=mock.DEFAULT,
        _ChromeOSController=mock.DEFAULT) as mock_classes:
      controller = profiling_mod.BrowserIntervalProfilingController(
          possible_browser, '', ['period1'], 1, [])
      with controller.SamplePeriod('period1', None):
        pass
      self.assertEqual(mock_classes['_AndroidController'].call_count, 0)
      self.assertEqual(mock_classes['_LinuxController'].call_count, 0)
      self.assertEqual(mock_classes['_ChromeOSController'].call_count, 0)
      self.assertIs(controller._platform_controller, None)

  def testUnsupportedPlatformWithInvalidSamplePeriod(self):
    possible_browser = FakePossibleBrowser(FakeWindowsPlatformBackend())
    profiling_mod = browser_interval_profiling_controller

    with mock.patch.multiple(
        'telemetry.internal.browser.browser_interval_profiling_controller',
        _AndroidController=mock.DEFAULT,
        _LinuxController=mock.DEFAULT,
        _ChromeOSController=mock.DEFAULT) as mock_classes:
      controller = profiling_mod.BrowserIntervalProfilingController(
          possible_browser, '', [], 1, [])
      with controller.SamplePeriod('period1', None):
        pass
      self.assertEqual(mock_classes['_AndroidController'].call_count, 0)
      self.assertEqual(mock_classes['_LinuxController'].call_count, 0)
      self.assertEqual(mock_classes['_ChromeOSController'].call_count, 0)
      self.assertIs(controller._platform_controller, None)
