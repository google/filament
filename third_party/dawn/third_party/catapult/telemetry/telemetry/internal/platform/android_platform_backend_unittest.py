# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import unittest
from unittest import mock

from telemetry import decorators
from telemetry.core import util
from telemetry.internal.platform import android_device
from telemetry.internal.platform import android_platform_backend
from telemetry.testing import system_stub

from devil.android import battery_utils
from devil.android import device_utils

class AndroidPlatformBackendTest(unittest.TestCase):
  def setUp(self):
    self._stubs = system_stub.Override(
        android_platform_backend,
        ['perf_control', 'thermal_throttle'])

    self.fix_adb_instability_patcher = mock.patch.object(
        android_platform_backend, '_FixPossibleAdbInstability')
    self.fix_adb_instability_patcher.start()

    self.battery_patcher = mock.patch.object(battery_utils, 'BatteryUtils')
    self.battery_patcher.start()

    def get_prop(name, cache=None):
      del cache  # unused
      return {'ro.product.cpu.abi': 'armeabi-v7a'}.get(name)

    self.device_patcher = mock.patch.multiple(
        device_utils.DeviceUtils,
        RunShellCommand = mock.MagicMock(return_value=[""]),
        HasRoot=mock.MagicMock(return_value=True),
        GetProp=mock.MagicMock(side_effect=get_prop))
    self.device_patcher.start()

  def tearDown(self):
    self._stubs.Restore()
    self.fix_adb_instability_patcher.stop()
    self.battery_patcher.stop()
    self.device_patcher.stop()

  @staticmethod
  def CreatePlatformBackendForTest():
    return android_platform_backend.AndroidPlatformBackend(
        android_device.AndroidDevice('12345'), True)

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testGetFriendlyOsVersionNameInPlatformTags(self):
    backend = self.CreatePlatformBackendForTest()
    android_os_versions = {
        'k': 'kitkat',
        'l': 'lollipop',
        'm': 'marshmallow',
        'n': 'nougat',
        'o': 'oreo',
        'p': 'pie',
    }
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    return_value='foo'):
      with mock.patch.object(
          backend, 'GetDeviceTypeName', return_value='gobo'):
        with mock.patch.object(backend,
                               'GetOSReleaseVersion',
                               return_value='10'):
          for version in android_os_versions:
            with mock.patch.object(backend,
                                   'GetOSVersionName',
                                   return_value=version):
              self.assertIn('android-' + android_os_versions[version],
                            backend.GetTypExpectationsTags())

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testTypExpectationsContainAndroidLetterPriorToCutoff(self):

    def side_effect(prop, *args, **kwargs):
      del args, kwargs
      if prop == 'ro.build.id':
        return 't'
      if prop == 'ro.build.version.release':
        return '13.0'
      return 'foo'

    backend = self.CreatePlatformBackendForTest()
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    side_effect=side_effect):
      tags = backend.GetTypExpectationsTags()
      self.assertIn('android-t', tags)
      self.assertIn('android-13', tags)

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testTypExpectationsMissingAndroidLetterAfterCutoff(self):

    def side_effect(prop, *args, **kwargs):
      del args, kwargs
      if prop == 'ro.build.id':
        return 'u'
      if prop == 'ro.build.version.release':
        return '14'
      return 'foo'

    backend = self.CreatePlatformBackendForTest()
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    side_effect=side_effect):
      tags = backend.GetTypExpectationsTags()
      self.assertNotIn('android-u', tags)
      self.assertIn('android-14', tags)

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testTypExpectationsTagsContainsLowEndTagForSvelteBuild(self):
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    return_value='svelte'):
      backend = self.CreatePlatformBackendForTest()
      with mock.patch.object(backend, 'GetOSReleaseVersion', return_value='10'):
        tags = backend.GetTypExpectationsTags()
        self.assertIn('android-low-end', tags)

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testTypExpectationsTagsIncludesTagsForAndroidGo(self):
    backend = self.CreatePlatformBackendForTest()
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    return_value='foo'):
      with mock.patch.object(backend, 'GetDeviceTypeName', return_value='gobo'):
        with mock.patch.object(backend,
                               'GetOSReleaseVersion',
                               return_value='10'):
          tags = backend.GetTypExpectationsTags()
          self.assertIn('android-low-end', tags)
          self.assertIn('android-go', tags)

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testTypExpectationsTagsIncludesTagsForAndroidOne(self):
    backend = self.CreatePlatformBackendForTest()
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    return_value='foo'):
      with mock.patch.object(backend, 'GetDeviceTypeName',
                             return_value='W6210'):
        with mock.patch.object(backend,
                               'GetOSReleaseVersion',
                               return_value='10'):
          tags = backend.GetTypExpectationsTags()
          self.assertIn('android-low-end', tags)
          self.assertIn('android-one', tags)

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testTypExpectationsDoesNotIncludeLowEndTagForNonSvelteBuild(self):
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    return_value='foo'):
      backend = self.CreatePlatformBackendForTest()
      with mock.patch.object(backend, 'GetOSReleaseVersion', return_value='10'):
        self.assertNotIn('android-low-end', backend.GetTypExpectationsTags())

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testTypExpectationsTagsIncludesForNexus6(self):
    backend = self.CreatePlatformBackendForTest()
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    return_value='foo'):
      with mock.patch.object(backend, 'GetDeviceTypeName',
                             return_value='AOSP on Shamu'):
        with mock.patch.object(backend,
                               'GetOSReleaseVersion',
                               return_value='10'):
          self.assertIn('android-nexus-6', backend.GetTypExpectationsTags())
          self.assertIn('mobile', backend.GetTypExpectationsTags())

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testTypExpectationsTagsIncludesForNexus5x(self):
    backend = self.CreatePlatformBackendForTest()
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    return_value='foo'):
      with mock.patch.object(backend, 'GetDeviceTypeName',
                             return_value='AOSP on BullHead'):
        with mock.patch.object(backend,
                               'GetOSReleaseVersion',
                               return_value='10'):
          self.assertIn('android-nexus-5x', backend.GetTypExpectationsTags())
          self.assertIn('mobile', backend.GetTypExpectationsTags())

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testIsSvelte(self):
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    return_value='svelte'):
      backend = self.CreatePlatformBackendForTest()
      self.assertTrue(backend.IsSvelte())

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testIsNotSvelte(self):
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    return_value='foo'):
      backend = self.CreatePlatformBackendForTest()
      self.assertFalse(backend.IsSvelte())

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testIsAosp(self):
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    return_value='aosp'):
      backend = self.CreatePlatformBackendForTest()
      self.assertTrue(backend.IsAosp())

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testIsNotAosp(self):
    with mock.patch('devil.android.device_utils.DeviceUtils.GetProp',
                    return_value='foo'):
      backend = self.CreatePlatformBackendForTest()
      self.assertFalse(backend.IsAosp())

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testIsScreenLockedTrue(self):
    test_input = ['a=b', 'mHasBeenInactive=true']
    backend = self.CreatePlatformBackendForTest()
    self.assertTrue(backend._IsScreenLocked(test_input))

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testIsScreenLockedFalse(self):
    test_input = ['a=b', 'mHasBeenInactive=false']
    backend = self.CreatePlatformBackendForTest()
    self.assertFalse(backend._IsScreenLocked(test_input))

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testPackageExtractionNotFound(self):
    backend = self.CreatePlatformBackendForTest()
    self.assertEqual(
        'com.google.android.apps.chrome',
        backend._ExtractLastNativeCrashPackageFromLogcat('no crash info here'))

  @staticmethod
  def GetExampleLogcat():
    test_file = os.path.join(util.GetUnittestDataDir(), 'crash_in_logcat.txt')
    with open(test_file) as f:
      return f.read()

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testPackageExtractionFromRealExample(self):
    backend = self.CreatePlatformBackendForTest()
    self.assertEqual(
        'com.google.android.apps.chrome',
        backend._ExtractLastNativeCrashPackageFromLogcat(
            self.GetExampleLogcat(), default_package_name='invalid'))

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testPackageExtractionWithProcessName(self):
    backend = self.CreatePlatformBackendForTest()
    test_file = os.path.join(util.GetUnittestDataDir(),
                             'crash_in_logcat_with_process_name.txt')
    with open(test_file) as f:
      logcat = f.read()
    self.assertEqual("org.chromium.chrome",
                     backend._ExtractLastNativeCrashPackageFromLogcat(logcat))

  @decorators.Disabled('chromeos', 'mac', 'win')
  def testPackageExtractionWithTwoCrashes(self):
    """Check that among two matches the latest package name is taken."""
    backend = self.CreatePlatformBackendForTest()
    original_logcat = self.GetExampleLogcat()
    mutated_logcat = original_logcat.replace('com.google.android.apps.chrome',
                                             'com.android.chrome')
    concatenated_logcat = '\n'.join([original_logcat, mutated_logcat])
    self.assertEqual(
        'com.android.chrome',
        backend._ExtractLastNativeCrashPackageFromLogcat(concatenated_logcat))


class AndroidPlatformBackendPsutilTest(unittest.TestCase):

  class psutil_1_0():
    version_info = (1, 0)
    def __init__(self):
      self.set_cpu_affinity_args = []
    class Process():
      def __init__(self, parent):
        self._parent = parent
        self.name = 'adb'
      def set_cpu_affinity(self, cpus):
        self._parent.set_cpu_affinity_args.append(cpus)
    def process_iter(self):
      return [self.Process(self)]

  class psutil_2_0():
    version_info = (2, 0)
    def __init__(self):
      self.set_cpu_affinity_args = []
    class Process():
      def __init__(self, parent):
        self._parent = parent
        self.set_cpu_affinity_args = []
      def name(self):
        return 'adb'
      def cpu_affinity(self, cpus=None):
        self._parent.set_cpu_affinity_args.append(cpus)
    def process_iter(self):
      return [self.Process(self)]

  def setUp(self):
    self._stubs = system_stub.Override(
        android_platform_backend,
        ['perf_control'])
    self.battery_patcher = mock.patch.object(battery_utils, 'BatteryUtils')
    self.battery_patcher.start()

    def get_prop(name, cache=None):
      del cache  # unused
      return {'ro.product.cpu.abi': 'armeabi-v7a'}.get(name)

    self.device_patcher = mock.patch.multiple(
        device_utils.DeviceUtils,
        FileExists=mock.MagicMock(return_value=False),
        GetProp=mock.MagicMock(side_effect=get_prop),
        HasRoot=mock.MagicMock(return_value=True))
    self.device_patcher.start()

  def tearDown(self):
    self._stubs.Restore()
    self.battery_patcher.stop()
    self.device_patcher.stop()
