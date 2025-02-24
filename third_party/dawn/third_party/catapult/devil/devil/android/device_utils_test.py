#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Unit tests for the contents of device_utils.py (mostly DeviceUtils).
"""

# pylint: disable=protected-access
# pylint: disable=unused-argument

import collections
import contextlib
import io
import json
import logging
import os
import posixpath
import stat
import sys
import unittest

from unittest import mock

import six

from devil import devil_env
from devil.android import device_errors
from devil.android import device_signal
from devil.android import device_utils
from devil.android.ndk import abis
from devil.android.sdk import adb_wrapper
from devil.android.sdk import intent
from devil.android.sdk import keyevent
from devil.android.sdk import version_codes
from devil.utils import cmd_helper
from devil.utils import mock_calls

with devil_env.SysPath(os.path.join(devil_env.PY_UTILS_PATH)):
  from py_utils import tempfile_ext

TEST_APK_PATH = '/fake/test/app.apk'
TEST_APEX_PATH = '/fake/test/module.apex'
TEST_PACKAGE = 'test.package'


def Process(name, pid, ppid='1'):
  return device_utils.ProcessInfo(name=name, pid=pid, ppid=ppid)


def Processes(*args):
  return [Process(*arg) for arg in args]


class AnyStringWith(object):
  def __init__(self, value):
    self._value = value

  def __eq__(self, other):
    return self._value in other

  def __repr__(self):
    return '<AnyStringWith: %s>' % self._value


class _FakeContextManager(object):
  def __init__(self, obj):
    self._obj = obj

  def __enter__(self):
    return self._obj

  def __exit__(self, type_, value, traceback):
    pass


class _MockApkHelper(object):
  def __init__(self, path, package_name, perms=None, splits=None):
    self.path = path
    self.is_bundle = path.endswith('_bundle')
    self.package_name = package_name
    self.perms = perms
    self.splits = splits if splits else []
    self.abis = [abis.ARM]
    self.version_code = None
    self.library_version = None

  def GetPackageName(self):
    return self.package_name

  def GetPermissions(self):
    return self.perms

  def GetLibraryVersion(self):
    return self.library_version

  def GetVersionCode(self):
    return self.version_code

  def GetAbis(self):
    return self.abis

  def GetApkPaths(self,
                  device,
                  modules=None,
                  allow_cached_props=False,
                  additional_locales=None):
    return _FakeContextManager([self.path] + self.splits)

  #override
  @staticmethod
  def SupportsSplits():
    return True


class _MockMultipleDevicesError(Exception):
  pass


class DeviceUtilsInitTest(unittest.TestCase):
  @mock.patch('devil.android.sdk.adb_wrapper.AdbWrapper.is_ready',
              return_value=True)
  def testInitWithStr(self, _mock_get_state):
    serial_as_str = str('0123456789abcdef')
    d = device_utils.DeviceUtils('0123456789abcdef')
    self.assertEqual(serial_as_str, d.adb.GetDeviceSerial())

  @mock.patch('devil.android.sdk.adb_wrapper.AdbWrapper.is_ready',
              return_value=True)
  def testInitWithUnicode(self, _mock_get_state):
    if six.PY2:
      serial_as_unicode = six.text_type('fedcba9876543210')
      d = device_utils.DeviceUtils(serial_as_unicode)
      self.assertEqual(serial_as_unicode, d.adb.GetDeviceSerial())

  @mock.patch('devil.android.sdk.adb_wrapper.AdbWrapper.is_ready',
              return_value=True)
  def testInitWithAdbWrapper(self, _mock_get_state):
    serial = '123456789abcdef0'
    a = adb_wrapper.AdbWrapper(serial)
    d = device_utils.DeviceUtils(a)
    self.assertEqual(serial, d.adb.GetDeviceSerial())

  def testInitWithMissing_fails(self):
    with self.assertRaises(ValueError):
      device_utils.DeviceUtils(None)
    with self.assertRaises(ValueError):
      device_utils.DeviceUtils('')


class DeviceUtilsGetAVDsTest(mock_calls.TestCase):
  def testGetAVDs(self):
    mocked_attrs = {'android_sdk': '/my/sdk/path'}
    with mock.patch('devil.devil_env._Environment.LocalPath',
                    mock.Mock(side_effect=lambda a: mocked_attrs[a])):
      with self.assertCall(
          mock.call.devil.utils.cmd_helper.GetCmdOutput(
              [mock.ANY, 'list', 'avd']), 'Available Android Virtual Devices:\n'
          '    Name: my_android5.0\n'
          '    Path: /some/path/to/.android/avd/my_android5.0.avd\n'
          '  Target: Android 5.0 (API level 21)\n'
          ' Tag/ABI: default/x86\n'
          '    Skin: WVGA800\n'):
        self.assertEqual(['my_android5.0'], device_utils.GetAVDs())


class DeviceUtilsRestartServerTest(mock_calls.TestCase):
  @mock.patch('time.sleep', mock.Mock())
  def testRestartServer_succeeds(self):
    with self.assertCalls(
        mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.KillServer(),
        (mock.call.devil.utils.cmd_helper.GetCmdStatusAndOutput(
            ['pgrep', 'adb']), (1, '')),
        mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.StartServer(),
        (mock.call.devil.utils.cmd_helper.GetCmdStatusAndOutput(
            ['pgrep', 'adb']),
         (1, '')), (mock.call.devil.utils.cmd_helper.GetCmdStatusAndOutput(
             ['pgrep', 'adb']), (0, '123\n'))):
      adb_wrapper.RestartServer()


class MockTempFile(object):
  def __init__(self, name='/tmp/some/file'):
    self.file = mock.MagicMock(spec=io.BufferedIOBase)
    self.file.name = name
    self.file.name_quoted = cmd_helper.SingleQuote(name)

  def __enter__(self):
    return self.file

  def __exit__(self, exc_type, exc_val, exc_tb):
    pass

  @property
  def name(self):
    return self.file.name


class MockLogger(mock.Mock):
  def __init__(self, *args, **kwargs):
    super(MockLogger, self).__init__(*args, **kwargs)
    self.warnings = []

  def warning(self, message, *args):
    self.warnings.append(message % args)


def PatchLogger():
  return mock.patch(
      'devil.android.device_utils.logger', new_callable=MockLogger)


class _PatchedFunction(object):
  def __init__(self, patched=None, mocked=None):
    self.patched = patched
    self.mocked = mocked


def _AdbWrapperMock(test_serial, is_ready=True):
  adb = mock.Mock(spec=adb_wrapper.AdbWrapper)
  adb.__str__ = mock.Mock(return_value=test_serial)
  adb.GetDeviceSerial.return_value = test_serial
  adb.is_ready = is_ready
  return adb


class DeviceUtilsTest(mock_calls.TestCase):
  def setUp(self):
    self.adb = _AdbWrapperMock('0123456789abcdef')
    self.device = device_utils.DeviceUtils(
        self.adb, default_timeout=10, default_retries=0)
    self.watchMethodCalls(self.call.adb, ignore=['GetDeviceSerial'])

  def safeAssertItemsEqual(self, expected, actual):
    six.assertCountEqual(self, expected, actual)

  def AdbCommandError(self, args=None, output=None, status=None, msg=None):
    if args is None:
      args = ['[unspecified]']
    return mock.Mock(
        side_effect=device_errors.AdbCommandFailedError(args, output, status,
                                                        msg, str(self.device)))

  def CommandError(self, msg=None):
    if msg is None:
      msg = 'Command failed'
    return mock.Mock(
        side_effect=device_errors.CommandFailedError(msg, str(self.device)))

  def ShellError(self, output=None, status=1):
    def action(cmd, *args, **kwargs):
      raise device_errors.AdbShellCommandFailedError(cmd, output, status,
                                                     str(self.device))

    if output is None:
      output = 'Permission denied\n'
    return action

  def TimeoutError(self, msg=None):
    if msg is None:
      msg = 'Operation timed out'
    return mock.Mock(
        side_effect=device_errors.CommandTimeoutError(msg, str(self.device)))

  def EnsureCacheInitialized(self, props=None, sdcard='/sdcard'):
    props = props or []
    ret = [sdcard, 'TOKEN'] + props
    return (self.call.device.RunShellCommand(
        AnyStringWith('getprop'),
        shell=True,
        check_return=True,
        large_output=True), ret)


class DeviceUtilsEqTest(DeviceUtilsTest):
  def testEq_equal_deviceUtils(self):
    other = device_utils.DeviceUtils(_AdbWrapperMock('0123456789abcdef'))
    self.assertTrue(self.device == other)
    self.assertTrue(other == self.device)

  @mock.patch('devil.android.sdk.adb_wrapper.AdbWrapper.is_ready',
              return_value=True)
  def testEq_equal_adbWrapper(self, _mock_get_state):
    other = adb_wrapper.AdbWrapper('0123456789abcdef')
    self.assertTrue(self.device == other)
    self.assertTrue(other == self.device)

  def testEq_equal_string(self):
    other = '0123456789abcdef'
    self.assertTrue(self.device == other)
    self.assertTrue(other == self.device)

  def testEq_devicesNotEqual(self):
    other = device_utils.DeviceUtils(_AdbWrapperMock('0123456789abcdee'))
    self.assertFalse(self.device == other)
    self.assertFalse(other == self.device)

  def testEq_identity(self):
    self.assertTrue(self.device == self.device)

  def testEq_serialInList(self):
    devices = [self.device]
    self.assertTrue('0123456789abcdef' in devices)


class DeviceUtilsLtTest(DeviceUtilsTest):
  def testLt_lessThan(self):
    other = device_utils.DeviceUtils(_AdbWrapperMock('ffffffffffffffff'))
    self.assertTrue(self.device < other)
    self.assertTrue(other > self.device)

  def testLt_greaterThan_lhs(self):
    other = device_utils.DeviceUtils(_AdbWrapperMock('0000000000000000'))
    self.assertFalse(self.device < other)
    self.assertFalse(other > self.device)

  def testLt_equal(self):
    other = device_utils.DeviceUtils(_AdbWrapperMock('0123456789abcdef'))
    self.assertFalse(self.device < other)
    self.assertFalse(other > self.device)

  def testLt_sorted(self):
    devices = [
        device_utils.DeviceUtils(_AdbWrapperMock('ffffffffffffffff')),
        device_utils.DeviceUtils(_AdbWrapperMock('0000000000000000')),
    ]
    sorted_devices = sorted(devices)
    self.assertEqual('0000000000000000',
                     sorted_devices[0].adb.GetDeviceSerial())
    self.assertEqual('ffffffffffffffff',
                     sorted_devices[1].adb.GetDeviceSerial())


class DeviceUtilsStrTest(DeviceUtilsTest):
  def testStr_returnsSerial(self):
    with self.assertCalls((self.call.adb.GetDeviceSerial(),
                           '0123456789abcdef')):
      self.assertEqual('0123456789abcdef', str(self.device))


class DeviceUtilsIsOnlineTest(DeviceUtilsTest):
  def testIsOnline_true(self):
    with self.assertCall(self.call.adb.GetState(), 'device'):
      self.assertTrue(self.device.IsOnline())

  def testIsOnline_false(self):
    with self.assertCall(self.call.adb.GetState(), 'offline'):
      self.assertFalse(self.device.IsOnline())

  def testIsOnline_error(self):
    with self.assertCall(self.call.adb.GetState(), self.CommandError()):
      self.assertFalse(self.device.IsOnline())


class DeviceUtilsHasRootTest(DeviceUtilsTest):
  def testHasRoot_true(self):
    with self.patch_call(self.call.device.build_type,
                         return_value='userdebug'), (self.assertCall(
                             self.call.adb.Shell('id'), 'uid=0(root)\n')):
      self.assertTrue(self.device.HasRoot())

  def testHasRootEngBuild_true(self):
    with self.patch_call(self.call.device.build_type, return_value='eng'):
      self.assertTrue(self.device.HasRoot())

  def testHasRoot_false(self):
    with self.patch_call(self.call.device.build_type,
                         return_value='userdebug'), (self.assertCall(
                             self.call.adb.Shell('id'), 'uid=2000(shell)\n')):
      self.assertFalse(self.device.HasRoot())


class DeviceUtilsEnableRootTest(DeviceUtilsTest):
  def testEnableRoot_succeeds(self):
    with self.assertCalls(self.call.adb.Root(), self.call.adb.WaitForDevice(),
                          (self.call.device.HasRoot(), True)):
      self.device.EnableRoot()

  def testEnableRoot_userBuild(self):
    with self.assertCalls((self.call.adb.Root(), self.AdbCommandError()),
                          (self.call.device.IsUserBuild(), True)):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.EnableRoot()

  def testEnableRoot_rootFails(self):
    with self.assertCalls((self.call.adb.Root(), self.AdbCommandError()),
                          (self.call.device.IsUserBuild(), False)):
      with self.assertRaises(device_errors.AdbCommandFailedError):
        self.device.EnableRoot()

  def testEnableRoot_timeoutInWaitForDevice(self):
    with self.assertCalls(
        (self.call.adb.Root(),
         self.AdbCommandError(
             output='timeout expired while waiting for device')),
        (self.call.device.IsUserBuild(), False), self.call.adb.WaitForDevice(),
        (self.call.device.HasRoot(), True)):
      self.device.EnableRoot()


class DeviceUtilsIsUserBuildTest(DeviceUtilsTest):
  def testIsUserBuild_yes(self):
    with self.assertCall(
        self.call.device.GetProp('ro.build.type', cache=True), 'user'):
      self.assertTrue(self.device.IsUserBuild())

  def testIsUserBuild_no(self):
    with self.assertCall(
        self.call.device.GetProp('ro.build.type', cache=True), 'userdebug'):
      self.assertFalse(self.device.IsUserBuild())


class DeviceUtilsGetExternalStoragePathTest(DeviceUtilsTest):
  def testGetExternalStoragePath_succeeds(self):
    with self.assertCalls(
        self.EnsureCacheInitialized(sdcard='/fake/storage/path')):
      self.assertEqual('/fake/storage/path',
                       self.device.GetExternalStoragePath())

  def testGetExternalStoragePath_fails(self):
    with self.assertCalls(self.EnsureCacheInitialized(sdcard='')):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.GetExternalStoragePath()


class DeviceUtilsGetAppWritablePathTest(DeviceUtilsTest):
  def testGetAppWritablePath_succeeds_sdk_pre_q(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '28'),
        self.EnsureCacheInitialized(sdcard='/fake/storage/path')):
      self.assertEqual('/fake/storage/path', self.device.GetAppWritablePath())

  def testGetAppWritablePath_succeeds_sdk_q(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '29'),
        self.EnsureCacheInitialized(sdcard='/fake/storage/path')):
      self.assertEqual('/fake/storage/path/Download',
                       self.device.GetAppWritablePath())

  def testGetAppWritablePath_fails(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '29'),
        self.EnsureCacheInitialized(sdcard='')):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.GetAppWritablePath()


class DeviceUtilsResolveSpecialPathTest(DeviceUtilsTest):
  def testResolveSpecialPath_user0(self):
    with self.patch_call(self.call.device.target_user, return_value=0):
      self.assertEqual('/system',
                       self.device.ResolveSpecialPath('/system'))
      self.assertEqual('/sdcard/foo',
                       self.device.ResolveSpecialPath('/sdcard/foo'))
      self.assertEqual('/data/data/bar',
                       self.device.ResolveSpecialPath('/data/data/bar'))

  def testResolveSpecialPath_user10(self):
    with self.patch_call(self.call.device.target_user, return_value=10):
      self.assertEqual('/system',
                       self.device.ResolveSpecialPath('/system'))
      self.assertEqual('/data/media/10/foo',
                       self.device.ResolveSpecialPath('/sdcard/foo'))
      self.assertEqual('/data/user/10/bar',
                       self.device.ResolveSpecialPath('/data/data/bar'))


class DeviceUtilsIsApplicationInstalledTest(DeviceUtilsTest):
  def testIsApplicationInstalled_installed(self):
    with self.assertCalls((self.call.device.RunShellCommand(
        ['pm', 'list', 'packages', 'some.installed.app'], check_return=True),
                           ['package:some.installed.app'])):
      self.assertTrue(self.device.IsApplicationInstalled('some.installed.app'))

  def testIsApplicationInstalled_installed_currentUser(self):
    with self.patch_call(self.call.device.target_user, return_value=11):
      with self.assertCalls((self.call.device.RunShellCommand(
          ['pm', 'list', 'packages', '--user', '11', 'some.installed.app'],
          check_return=True), ['package:some.installed.app'])):
        self.assertTrue(
            self.device.IsApplicationInstalled('some.installed.app'))

  def testIsApplicationInstalled_notInstalled(self):
    with self.assertCalls((self.call.device.RunShellCommand(
        ['pm', 'list', 'packages', 'not.installed.app'],
        check_return=True), ''), (self.call.device.RunShellCommand(
            ['dumpsys', 'package', 'not.installed.app'],
            check_return=True,
            large_output=True), [])):
      self.assertFalse(self.device.IsApplicationInstalled('not.installed.app'))

  def testIsApplicationInstalled_substringMatch(self):
    with self.assertCalls(
        (self.call.device.RunShellCommand(
            ['pm', 'list', 'packages', 'substring.of.package'],
            check_return=True), [
                'package:first.substring.of.package',
                'package:second.substring.of.package',
            ]), (self.call.device.RunShellCommand(
                ['dumpsys', 'package', 'substring.of.package'],
                check_return=True,
                large_output=True), [])):
      self.assertFalse(
          self.device.IsApplicationInstalled('substring.of.package'))

  def testIsApplicationInstalled_dumpsysFallback(self):
    with self.assertCalls((self.call.device.RunShellCommand(
        ['pm', 'list', 'packages', 'some.installed.app'],
        check_return=True), []), (self.call.device.RunShellCommand(
            ['dumpsys', 'package', 'some.installed.app'],
            check_return=True,
            large_output=True), ['Package [some.installed.app] (a12345):'])):
      self.assertTrue(self.device.IsApplicationInstalled('some.installed.app'))

  def testIsApplicationInstalled_dumpsysFallbackVersioned(self):
    with self.assertCalls((self.call.device.RunShellCommand(
        ['dumpsys', 'package', 'some.installed.app_1234'],
        check_return=True,
        large_output=True), ['Package [some.installed.app_1234] (a1245):'])):
      self.assertTrue(
          self.device.IsApplicationInstalled('some.installed.app', 1234))

  def testIsApplicationInstalled_dumpsysFallbackVersioned_currentUserInstalled(
      self):
    dumpsys_output = [
        'Package [some.installed.app_1234] (a1245):',
        '  User 0: ceDataInode=0 installed=false hidden=false suspended=false',
        '  User 10: ceDataInode=0 installed=false hidden=false suspended=false',
        '  User 11: ceDataInode=0 installed=true hidden=false suspended=false',
    ]
    with self.patch_call(self.call.device.target_user, return_value=11):
      with self.assertCalls((self.call.device.RunShellCommand(
          ['dumpsys', 'package', 'some.installed.app_1234'],
          check_return=True,
          large_output=True), dumpsys_output)):
        self.assertTrue(
            self.device.IsApplicationInstalled('some.installed.app', 1234))

  def testIsApplicationInstalled_dumpsysFallbackVersioned_currentUserNotInstalled(
      self):
    dumpsys_output = [
        'Package [some.installed.app_1234] (a1245):',
        '  User 0: ceDataInode=0 installed=true hidden=false suspended=false',
        '  User 10: ceDataInode=0 installed=false hidden=false suspended=false',
        '  User 11: ceDataInode=0 installed=false hidden=false suspended=false',
    ]
    with self.patch_call(self.call.device.target_user, return_value=11):
      with self.assertCalls((self.call.device.RunShellCommand(
          ['dumpsys', 'package', 'some.installed.app_1234'],
          check_return=True,
          large_output=True), dumpsys_output)):
        self.assertFalse(
            self.device.IsApplicationInstalled('some.installed.app', 1234))

  def testIsApplicationInstalled_dumpsysFallbackVersionNotInstalled(self):
    with self.assertCalls((self.call.device.RunShellCommand(
        ['dumpsys', 'package', 'some.installed.app_1234'],
        check_return=True,
        large_output=True), ['Package [some.installed.app_2000] (a1245):'])):
      self.assertFalse(
          self.device.IsApplicationInstalled('some.installed.app', 1234))


class DeviceUtilsIsSystemModuleInstalledTest(DeviceUtilsTest):
  def testIsSystemModuleInstalled_installed(self):
    with self.assertCalls((self.call.device.RunShellCommand(
        ['dumpsys', 'package', 'some.installed.module'],
        check_return=True,
        large_output=True), ['Some app info', ' Version: 1234', 'extra info'])):
      self.assertTrue(
          self.device.IsSystemModuleInstalled('some.installed.module', 1234))

  def testIsSystemModuleInstalled_installedWrongVersion(self):
    with self.assertCalls((self.call.device.RunShellCommand(
        ['dumpsys', 'package', 'some.installed.module'],
        check_return=True,
        large_output=True), ['Some app info', ' Version: 3284', 'extra info'])):
      self.assertFalse(
          self.device.IsSystemModuleInstalled('some.installed.module', 1234))

  def testIsSystemModuleInstalled_NotInstalled(self):
    with self.assertCalls((self.call.device.RunShellCommand(
        ['dumpsys', 'package', 'some.installed.module'],
        check_return=True,
        large_output=True), [])):
      self.assertFalse(
          self.device.IsSystemModuleInstalled('some.installed.module', 1234))


class DeviceUtilsGetApplicationPathsInternalTest(DeviceUtilsTest):
  def testGetApplicationPathsInternal_exists(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '19'),
        (self.call.device.RunShellCommand(['pm', 'path', 'android'],
                                          check_return=True),
         ['package:/path/to/android.apk'])):
      self.assertEqual(['/path/to/android.apk'],
                       self.device._GetApplicationPathsInternal('android'))

  def testGetApplicationPathsInternal_notExists(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '19'),
        (self.call.device.RunShellCommand(['pm', 'path', 'not.installed.app'],
                                          check_return=True), '')):
      self.assertEqual(
          [], self.device._GetApplicationPathsInternal('not.installed.app'))

  def testGetApplicationPathsInternal_garbageOutputRaises(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '19'),
        (self.call.device.RunShellCommand(['pm', 'path', 'android'],
                                          check_return=True),
         ['garbage first line'])):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device._GetApplicationPathsInternal('android')

  def testGetApplicationPathsInternal_outputWarningsIgnored(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '19'),
        (self.call.device.RunShellCommand(['pm', 'path', 'not.installed.app'],
                                          check_return=True),
         ['WARNING: some warning message from pm'])):
      self.assertEqual(
          [], self.device._GetApplicationPathsInternal('not.installed.app'))

  def testGetApplicationPathsInternal_fails(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '19'),
        (self.call.device.RunShellCommand(['pm', 'path', 'android'],
                                          check_return=True),
         self.CommandError('ERROR. Is package manager running?\n'))):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device._GetApplicationPathsInternal('android')


class DeviceUtils_GetApplicationVersionTest(DeviceUtilsTest):
  def test_GetApplicationVersion_exists(self):
    with self.assertCall(
        self.call.device._GetDumpsysOutput(['package', 'com.android.chrome'],
                                           'versionName='),
        ['    versionName=45.0.1234.7']):
      self.assertEqual('45.0.1234.7',
                       self.device.GetApplicationVersion('com.android.chrome'))

  def test_GetApplicationVersion_notExists(self):
    with self.assertCalls(
        (self.call.device._GetDumpsysOutput(['package', 'com.android.chrome'],
                                            'versionName='), [''])):
      self.assertEqual(None,
                       self.device.GetApplicationVersion('com.android.chrome'))

  def test_GetApplicationVersion_fails(self):
    with self.assertCalls(
        (self.call.device._GetDumpsysOutput(['package', 'com.android.chrome'],
                                            'versionName='), [])):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.GetApplicationVersion('com.android.chrome')


class DeviceUtils_GetApplicationTargetSdkTest(DeviceUtilsTest):
  def test_GetApplicationTargetSdk_exists(self):
    with self.assertCalls(
        (self.call.device.IsApplicationInstalled('com.android.chrome'), True),
        (self.call.device._GetDumpsysOutput(['package', 'com.android.chrome'],
                                            'targetSdk='),
         ['    versionCode=413200001 minSdk=21 targetSdk=29'])):
      self.assertEqual(
          '29', self.device.GetApplicationTargetSdk('com.android.chrome'))

  def test_GetApplicationTargetSdk_notExists(self):
    with self.assertCalls(
        (self.call.device.IsApplicationInstalled('com.android.chrome'), False)):
      self.assertIsNone(
          self.device.GetApplicationTargetSdk('com.android.chrome'))

  def test_GetApplicationTargetSdk_fails(self):
    with self.assertCalls(
        (self.call.device.IsApplicationInstalled('com.android.chrome'), True),
        (self.call.device._GetDumpsysOutput(['package', 'com.android.chrome'],
                                            'targetSdk='), [])):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.GetApplicationTargetSdk('com.android.chrome')

  def test_GetApplicationTargetSdk_prefinalizedSdk(self):
    with self.assertCalls(
        (self.call.device.IsApplicationInstalled('com.android.chrome'), True),
        (self.call.device._GetDumpsysOutput(['package', 'com.android.chrome'],
                                            'targetSdk='),
         ['    versionCode=410301483 minSdk=10000 targetSdk=10000']),
        (self.call.device.GetProp('ro.build.version.codename',
                                  cache=True), 'R')):
      self.assertEqual(
          'R', self.device.GetApplicationTargetSdk('com.android.chrome'))


class DeviceUtils_GetWebViewProvider(DeviceUtilsTest):
  def test_GetWebViewProvider(self):
    with self.assertCalls(
        (self.call.device._CheckSdkLevel(version_codes.NOUGAT), None),
        (self.call.device.RunShellCommand(
            ['settings', 'list', device_utils.SettingsNamespace.GLOBAL],
            check_return=True,
            large_output=True), ['a=b', 'webview_provider=com.android.webview'
                                 ])):
      self.assertEqual('com.android.webview', self.device.GetWebViewProvider())


class DeviceUtils_GetUidForPackageTest(DeviceUtilsTest):
  def test_GetUidForPackage_Exists(self):
    with self.assertCall(
        self.call.device._GetDumpsysOutput(
            ['package', 'com.android.chrome'], 'userId='),
        ['  userId=1001']):
      self.assertEqual('1001',
                       self.device.GetUidForPackage('com.android.chrome'))

  def test_GetUidForPackage_notInstalled(self):
    with self.assertCall(
        self.call.device._GetDumpsysOutput(
            ['package', 'com.android.chrome'], 'userId='),
        ['']):
      self.assertEqual(None, self.device.GetUidForPackage('com.android.chrome'))

  def test_GetUidForPackage_fails(self):
    with self.assertCall(
        self.call.device._GetDumpsysOutput(
            ['package', 'com.android.chrome'], 'userId='),
        []):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.GetUidForPackage('com.android.chrome')


class DeviceUtils_GetPackageArchitectureTest(DeviceUtilsTest):
  def test_GetPackageArchitecture_exists(self):
    with self.assertCall(
        self.call.device._RunPipedShellCommand(
            'dumpsys package com.android.chrome | grep -F primaryCpuAbi'),
        ['  primaryCpuAbi=armeabi-v7a']):
      self.assertEqual(abis.ARM,
                       self.device.GetPackageArchitecture('com.android.chrome'))

  def test_GetPackageArchitecture_notExists(self):
    with self.assertCall(
        self.call.device._RunPipedShellCommand(
            'dumpsys package com.android.chrome | grep -F primaryCpuAbi'), []):
      self.assertEqual(None,
                       self.device.GetPackageArchitecture('com.android.chrome'))


class DeviceUtilsGetApplicationDataDirectoryTest(DeviceUtilsTest):
  def testGetApplicationDataDirectory_exists(self):
    with self.assertCalls(
        (self.call.device.IsApplicationInstalled('foo.bar.baz'), True),
        (self.call.device.PathExists('/data/user/0/foo.bar.baz',
                                     as_root=True), True)):
      self.assertEqual('/data/user/0/foo.bar.baz',
                       self.device.GetApplicationDataDirectory('foo.bar.baz'))

  def testGetApplicationDataDirectory_exists_currentUser(self):
    with self.patch_call(self.call.device.target_user, return_value=11):
      with self.assertCalls(
          (self.call.device.IsApplicationInstalled('foo.bar.baz'), True),
          (self.call.device.PathExists('/data/user/11/foo.bar.baz',
                                       as_root=True), True)):
        self.assertEqual('/data/user/11/foo.bar.baz',
                         self.device.GetApplicationDataDirectory('foo.bar.baz'))

  def testGetApplicationDataDirectory_notInstalled(self):
    with self.assertCalls(
        (self.call.device.IsApplicationInstalled('foo.bar.baz'), False)):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.GetApplicationDataDirectory('foo.bar.baz')

  def testGetApplicationDataDirectory_notExists(self):
    with self.assertCalls(
        (self.call.device.IsApplicationInstalled('foo.bar.baz'), True),
        (self.call.device.PathExists('/data/user/0/foo.bar.baz',
                                     as_root=True), False)):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.GetApplicationDataDirectory('foo.bar.baz')


@mock.patch('time.sleep', mock.Mock())
class DeviceUtilsWaitUntilFullyBootedTest(DeviceUtilsTest):
  def testWaitUntilFullyBooted_succeedsWithDefaults(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), ['package:/some/fake/path'])):
      self.device.WaitUntilFullyBooted(wifi=False, decrypt=False)

  @mock.patch('devil.android.sdk.adb_wrapper.RestartServer', return_value=None)
  def testWaitUntilFullyBooted_succeedsAfterCommandTimeout(
      self, restart_server_mock):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), self.TimeoutError()),
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), ['package:/some/fake/path'])):
      self.device.WaitUntilFullyBooted(wifi=False, decrypt=False)
      self.assertEqual(restart_server_mock.call_count, 1)

  @mock.patch('devil.android.sdk.adb_wrapper.RestartServer', return_value=None)
  def testWaitUntilFullyBooted_failsAfterCommandTimeout(self,
                                                        restart_server_mock):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), self.TimeoutError()),
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), self.TimeoutError())):
      with self.assertRaises(device_errors.CommandTimeoutError):
        self.device.WaitUntilFullyBooted(wifi=False, decrypt=False, retries=1)
      self.assertEqual(restart_server_mock.call_count, 1)

  def testWaitUntilFullyBooted_succeedsWithWifi(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), ['package:/some/fake/path']),
        # wifi_enabled
        (self.call.adb.Shell('dumpsys wifi'),
         'stuff\nWi-Fi is enabled\nmore stuff\n')):
      self.device.WaitUntilFullyBooted(wifi=True, decrypt=False)

  def testWaitUntilFullyBooted_succeedsWithDecryptFDE(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), ['package:/some/fake/path']),
        # decryption_completed
        (self.call.device.GetProp('vold.decrypt',
                                  cache=False), 'trigger_restart_framework')):
      self.device.WaitUntilFullyBooted(wifi=False, decrypt=True)

  def testWaitUntilFullyBooted_succeedsWithDecryptNotFDE(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), ['package:/some/fake/path']),
        # decryption_completed
        (self.call.device.GetProp('vold.decrypt', cache=False), '')):
      self.device.WaitUntilFullyBooted(wifi=False, decrypt=True)

  def testWaitUntilFullyBooted_deviceIsRock960(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), 'rk3399'),
        (self.call.device.GetProp('sys.usb.config'), 'mtp,adb'),
        (self.call.device.GetProp('ro.product.model'), 'rk3399'),
        (self.call.device.GetProp('sys.usb.config'), 'adb'),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), ['package:/some/fake/path'])):
      self.device.WaitUntilFullyBooted(wifi=False, decrypt=False)

  def testWaitUntilFullyBooted_deviceNotInitiallyAvailable(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), self.AdbCommandError()),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), self.AdbCommandError()),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), self.AdbCommandError()),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), self.AdbCommandError()),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), ['package:/some/fake/path'])):
      self.device.WaitUntilFullyBooted(wifi=False, decrypt=False)

  def testWaitUntilFullyBooted_deviceBrieflyOffline(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed',
                                  cache=False), self.AdbCommandError()),
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), ['package:/some/fake/path'])):
      self.device.WaitUntilFullyBooted(wifi=False, decrypt=False)

  def testWaitUntilFullyBooted_sdCardReadyFails_noPath(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), self.CommandError()),
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), self.CommandError())):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.WaitUntilFullyBooted(wifi=False, decrypt=False, retries=1)

  def testWaitUntilFullyBooted_sdCardReadyFails_notExists(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), self.ShellError()),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), self.ShellError()),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'),
         self.TimeoutError())):
      with self.assertRaises(device_errors.CommandTimeoutError):
        self.device.WaitUntilFullyBooted(wifi=False, decrypt=False, retries=0)

  def testWaitUntilFullyBooted_devicePmFails(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), self.CommandError()),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), self.CommandError()),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), self.TimeoutError())):
      with self.assertRaises(device_errors.CommandTimeoutError):
        self.device.WaitUntilFullyBooted(wifi=False, decrypt=False, retries=0)

  def testWaitUntilFullyBooted_bootFails(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '0'),
        (self.call.device.GetProp('dev.bootcomplete', cache=False), '0'),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '0'),
        (self.call.device.GetProp('dev.bootcomplete', cache=False), '0'),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed',
                                  cache=False), self.TimeoutError())):
      with self.assertRaises(device_errors.CommandTimeoutError):
        self.device.WaitUntilFullyBooted(wifi=False, decrypt=False, retries=0)

  def testWaitUntilFullyBooted_wifiFails(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), ['package:/some/fake/path']),
        # wifi_enabled
        (self.call.adb.Shell('dumpsys wifi'), 'stuff\nmore stuff\n'),
        # wifi_enabled
        (self.call.adb.Shell('dumpsys wifi'), 'stuff\nmore stuff\n'),
        # wifi_enabled
        (self.call.adb.Shell('dumpsys wifi'), self.TimeoutError())):
      with self.assertRaises(device_errors.CommandTimeoutError):
        self.device.WaitUntilFullyBooted(wifi=True, decrypt=False, retries=0)

  def testWaitUntilFullyBooted_decryptFails(self):
    with self.assertCalls(
        self.call.adb.WaitForDevice(),
        # is_boot_completed
        (self.call.device.GetProp('sys.boot_completed', cache=False), '1'),
        # is_device_connection_ready
        (self.call.device.GetProp('ro.product.model'), ''),
        # sd_card_ready
        (self.call.device.GetExternalStoragePath(), '/fake/storage/path'),
        (self.call.adb.Shell('test -d /fake/storage/path'), ''),
        # pm_ready
        (self.call.device._GetApplicationPathsInternal(
            'android', skip_cache=True), ['package:/some/fake/path']),
        # decryption_completed
        (self.call.device.GetProp(
            'vold.decrypt', cache=False), 'trigger_restart_min_framework'),
        # decryption_completed
        (self.call.device.GetProp(
            'vold.decrypt', cache=False), 'trigger_restart_min_framework'),
        # decryption_completed
        (self.call.device.GetProp('vold.decrypt',
                                  cache=False), self.TimeoutError())):
      with self.assertRaises(device_errors.CommandTimeoutError):
        self.device.WaitUntilFullyBooted(wifi=False, decrypt=True, retries=0)


@mock.patch('time.sleep', mock.Mock())
class DeviceUtilsRebootTest(DeviceUtilsTest):
  def testReboot_nonBlocking(self):
    with self.assertCalls(self.call.adb.Reboot(),
                          (self.call.device.IsOnline(), True),
                          (self.call.device.IsOnline(), False)):
      self.device.Reboot(block=False)

  def testReboot_blocking(self):
    with self.assertCalls(
        (self.call.device.HasRoot(), False),
        self.call.adb.Reboot(), (self.call.device.IsOnline(), True),
        (self.call.device.IsOnline(), False),
        self.call.device.WaitUntilFullyBooted(wifi=False, decrypt=False)):
      self.device.Reboot(block=True)

  def testReboot_blockingWithRoot(self):
    with self.assertCalls(
        (self.call.device.HasRoot(), True),
        self.call.adb.Reboot(), (self.call.device.IsOnline(), True),
        (self.call.device.IsOnline(), False),
        self.call.device.WaitUntilFullyBooted(wifi=False, decrypt=False),
        self.call.device.EnableRoot()):
      self.device.Reboot(block=True)

  def testReboot_blockUntilWifi(self):
    with self.assertCalls(
        (self.call.device.HasRoot(), False),
        self.call.adb.Reboot(), (self.call.device.IsOnline(), True),
        (self.call.device.IsOnline(), False),
        self.call.device.WaitUntilFullyBooted(wifi=True, decrypt=False)):
      self.device.Reboot(block=True, wifi=True, decrypt=False)

  def testReboot_blockUntilDecrypt(self):
    with self.assertCalls(
        (self.call.device.HasRoot(), False),
        self.call.adb.Reboot(), (self.call.device.IsOnline(), True),
        (self.call.device.IsOnline(), False),
        self.call.device.WaitUntilFullyBooted(wifi=False, decrypt=True)):
      self.device.Reboot(block=True, wifi=False, decrypt=True)


class DeviceUtilsInstallTest(DeviceUtilsTest):

  mock_apk = _MockApkHelper(TEST_APK_PATH, TEST_PACKAGE, ['p1'])

  def testInstall_noPriorInstall(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False), \
         self.patch_call(self.call.device.build_version_sdk,
                         return_value=version_codes.NOUGAT):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          self.call.adb.Install(TEST_APK_PATH,
                                reinstall=False,
                                streaming=None,
                                allow_downgrade=False,
                                instant_app=False,
                                force_queryable=False),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True),
          (self.call.device.GrantPermissions(TEST_PACKAGE, ['p1']), [])):
        self.device.Install(DeviceUtilsInstallTest.mock_apk, retries=0)

  def testInstall_noStreaming_device(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='flounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False), \
         self.patch_call(self.call.device.build_version_sdk,
                         return_value=version_codes.NOUGAT):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          self.call.adb.Install(TEST_APK_PATH,
                                reinstall=False,
                                streaming=False,
                                allow_downgrade=False,
                                instant_app=False,
                                force_queryable=False),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True),
          (self.call.device.GrantPermissions(TEST_PACKAGE, ['p1']), [])):
        self.device.Install(DeviceUtilsInstallTest.mock_apk, retries=0)

  def testInstall_noStreaming_emulator(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=True), \
         self.patch_call(self.call.device.build_version_sdk,
                         return_value=version_codes.NOUGAT):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          self.call.adb.Install(TEST_APK_PATH,
                                reinstall=False,
                                streaming=False,
                                allow_downgrade=False,
                                instant_app=False,
                                force_queryable=False),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True),
          (self.call.device.GrantPermissions(TEST_PACKAGE, ['p1']), [])):
        self.device.Install(DeviceUtilsInstallTest.mock_apk, retries=0)

  def testInstall_permissionsPreM(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False), \
         self.patch_call(self.call.device.build_version_sdk,
                         return_value=version_codes.KITKAT_WATCH):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          (self.call.adb.Install(TEST_APK_PATH,
                                 reinstall=False,
                                 streaming=None,
                                 allow_downgrade=False,
                                 instant_app=False,
                                 force_queryable=False)),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.Install(DeviceUtilsInstallTest.mock_apk, retries=0)

  def testInstall_findPermissions(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False), \
         self.patch_call(self.call.device.build_version_sdk,
                         return_value=version_codes.NOUGAT):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          (self.call.adb.Install(TEST_APK_PATH,
                                 reinstall=False,
                                 streaming=None,
                                 allow_downgrade=False,
                                 instant_app=False,
                                 force_queryable=False)),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True),
          (self.call.device.GrantPermissions(TEST_PACKAGE, ['p1']), [])):
        self.device.Install(DeviceUtilsInstallTest.mock_apk, retries=0)

  def testInstall_passPermissions(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          (self.call.adb.Install(TEST_APK_PATH,
                                 reinstall=False,
                                 streaming=None,
                                 allow_downgrade=False,
                                 instant_app=False,
                                 force_queryable=False)),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True),
          (self.call.device.GrantPermissions(TEST_PACKAGE, ['p1', 'p2']), [])):
        self.device.Install(
            DeviceUtilsInstallTest.mock_apk,
            retries=0,
            permissions=['p1', 'p2'])

  def testInstall_identicalPriorInstall(self):
    with self.assertCalls(
        (self.call.device._FakeInstall(set(), None, 'test.package')),
        (mock.call.os.path.exists(TEST_APK_PATH), True),
        (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE),
         ['/fake/data/app/test.package.apk']),
        (self.call.device._ComputeStaleApks(TEST_PACKAGE, [TEST_APK_PATH]),
         ([], None)), (self.call.device.ClearApplicationState(TEST_PACKAGE)),
        (self.call.device.ForceStop(TEST_PACKAGE)),
        (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
      self.device.Install(
          DeviceUtilsInstallTest.mock_apk, retries=0, permissions=[])

  def testInstall_differentPriorInstall(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE),
           ['/fake/data/app/test.package.apk']),
          (self.call.device._ComputeStaleApks(TEST_PACKAGE, [TEST_APK_PATH]),
           ([TEST_APK_PATH], None)), self.call.device.Uninstall(TEST_PACKAGE),
          self.call.adb.Install(TEST_APK_PATH,
                                reinstall=False,
                                streaming=None,
                                allow_downgrade=False,
                                instant_app=False,
                                force_queryable=False),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.Install(
            DeviceUtilsInstallTest.mock_apk, retries=0, permissions=[])

  def testInstall_differentPriorInstallSplitApk(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), [
              '/fake/data/app/test.package.apk',
              '/fake/data/app/test.package2.apk'
          ]), self.call.device.Uninstall(TEST_PACKAGE),
          self.call.adb.Install(TEST_APK_PATH,
                                reinstall=False,
                                streaming=None,
                                allow_downgrade=False,
                                instant_app=False,
                                force_queryable=False),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.Install(
            DeviceUtilsInstallTest.mock_apk, retries=0, permissions=[])

  def testInstall_differentPriorInstall_reinstall(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE),
           ['/fake/data/app/test.package.apk']),
          (self.call.device._ComputeStaleApks(TEST_PACKAGE, [TEST_APK_PATH]),
           ([TEST_APK_PATH], None)),
          self.call.adb.Install(TEST_APK_PATH,
                                reinstall=True,
                                streaming=None,
                                allow_downgrade=False,
                                instant_app=False,
                                force_queryable=False),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.Install(
            DeviceUtilsInstallTest.mock_apk,
            reinstall=True,
            retries=0,
            permissions=[])

  def testInstall_identicalPriorInstall_reinstall(self):
    with self.assertCalls(
        (self.call.device._FakeInstall(set(), None, 'test.package')),
        (mock.call.os.path.exists(TEST_APK_PATH), True),
        (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE),
         ['/fake/data/app/test.package.apk']),
        (self.call.device._ComputeStaleApks(TEST_PACKAGE, [TEST_APK_PATH]),
         ([], None)), (self.call.device.ForceStop(TEST_PACKAGE)),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
      self.device.Install(
          DeviceUtilsInstallTest.mock_apk,
          reinstall=True,
          retries=0,
          permissions=[])

  def testInstall_missingApk(self):
    with self.assertCalls(
        (self.call.device._FakeInstall(set(), None, 'test.package')),
        (mock.call.os.path.exists(TEST_APK_PATH), False)):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.Install(DeviceUtilsInstallTest.mock_apk, retries=0)

  def testInstall_fails(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          (self.call.adb.Install(
              TEST_APK_PATH,
              reinstall=False,
              streaming=None,
              allow_downgrade=False,
              instant_app=False,
              force_queryable=False), self.CommandError('Failure\r\n'))):
        with self.assertRaises(device_errors.CommandFailedError):
          self.device.Install(DeviceUtilsInstallTest.mock_apk, retries=0)

  def testInstall_downgrade(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE),
           ['/fake/data/app/test.package.apk']),
          (self.call.device._ComputeStaleApks(TEST_PACKAGE, [TEST_APK_PATH]),
           ([TEST_APK_PATH], None)),
          self.call.adb.Install(TEST_APK_PATH,
                                reinstall=True,
                                streaming=None,
                                allow_downgrade=True,
                                instant_app=False,
                                force_queryable=False),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.Install(
            DeviceUtilsInstallTest.mock_apk,
            reinstall=True,
            retries=0,
            permissions=[],
            allow_downgrade=True)

  def testInstall_pushesFakeModulesToDevice(self):
    @contextlib.contextmanager
    def mock_zip_temp_dir():
      yield '/test/tmp/dir'

    mock_apk_with_fake = _MockApkHelper(
        TEST_APK_PATH, TEST_PACKAGE, splits=['fake1-master.apk'])
    fake_modules = ['fake1']
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False), \
         self.patch_call(self.call.device.build_version_sdk,
                         return_value=version_codes.NOUGAT):
      with self.assertCalls(
          (mock.call.py_utils.tempfile_ext.NamedTemporaryDirectory(),
           mock_zip_temp_dir),
          self.call.device.RunShellCommand([
              'rm', '-rf',
              '/sdcard/Android/data/test.package/files/local_testing'
          ],
                                           as_root=True),
          (mock.call.os.rename('fake1-master.apk', '/test/tmp/dir/fake1.apk')),
          (self.call.device.PushChangedFiles(
              [('/test/tmp/dir', '/data/local/tmp/modules/test.package')],
              delete_device_stale=True)),
          self.call.device.RunShellCommand([
              'mkdir', '-p',
              '/sdcard/Android/data/test.package/files/local_testing'
          ],
                                           as_root=True),
          self.call.device.RunShellCommand(
              'cp -a /data/local/tmp/modules/test.package/* ' +
              '/sdcard/Android/data/test.package/files/local_testing/',
              as_root=True,
              shell=True), (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          self.call.adb.Install(TEST_APK_PATH,
                                reinstall=False,
                                streaming=None,
                                allow_downgrade=False,
                                instant_app=False,
                                force_queryable=False),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True),
          (self.call.device.GrantPermissions(TEST_PACKAGE, None), [])):
        self.device.Install(
            mock_apk_with_fake, fake_modules=fake_modules, retries=0)

  def testInstall_packageNotAvailableAfterInstall(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False), \
         self.patch_call(self.call.device.build_version_sdk,
                         return_value=version_codes.NOUGAT), \
         self.patch_call(self.call.device.IsApplicationInstalled,
                         return_value=False):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          self.call.adb.Install(TEST_APK_PATH,
                                reinstall=False,
                                streaming=None,
                                allow_downgrade=False,
                                instant_app=False,
                                force_queryable=False)):
        with six.assertRaisesRegex(
            self, device_errors.CommandFailedError,
            'not installed on device after explicit install attempt'):
          self.device.Install(
              DeviceUtilsInstallTest.mock_apk, retries=0)

  def testInstall_instantApp(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False), \
         self.patch_call(self.call.device.build_version_sdk,
                         return_value=version_codes.NOUGAT):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          self.call.adb.Install(TEST_APK_PATH,
                                reinstall=False,
                                streaming=None,
                                allow_downgrade=False,
                                instant_app=True,
                                force_queryable=False),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True),
          (self.call.device.GrantPermissions(TEST_PACKAGE, ['p1']), [])):
        self.device.Install(DeviceUtilsInstallTest.mock_apk, instant_app=True)

  def testInstall_forceQueryable(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False), \
         self.patch_call(self.call.device.build_version_sdk,
                         return_value=version_codes.NOUGAT):
      with self.assertCalls(
          (self.call.device._FakeInstall(set(), None, 'test.package')),
          (mock.call.os.path.exists(TEST_APK_PATH), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          self.call.adb.Install(TEST_APK_PATH,
                                reinstall=False,
                                streaming=None,
                                allow_downgrade=False,
                                instant_app=False,
                                force_queryable=True),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True),
          (self.call.device.GrantPermissions(TEST_PACKAGE, ['p1']), [])):
        self.device.Install(DeviceUtilsInstallTest.mock_apk,
                            force_queryable=True)


class DeviceUtilsInstallApexTest(DeviceUtilsTest):

  mock_apex = _MockApkHelper(TEST_APEX_PATH, TEST_PACKAGE, ['p1'])

  def testInstallApex(self):
    with self.patch_call(self.call.device.build_version_sdk, return_value=29):
      with self.assertCalls(
          (mock.call.os.path.exists(TEST_APEX_PATH), True),
          self.call.adb.Install(TEST_APEX_PATH), self.call.device.Reboot(),
          (self.call.device.IsSystemModuleInstalled(TEST_PACKAGE, None), True)):
        self.device.InstallApex(DeviceUtilsInstallApexTest.mock_apex, retries=0)

  def testInstallApex_preAndroidQFails(self):
    with self.patch_call(self.call.device.build_version_sdk, return_value=28):
      with self.assertRaises(device_errors.DeviceVersionError):
        self.device.InstallApex(DeviceUtilsInstallApexTest.mock_apex, retries=0)

  def testInstallApex_fileDoesNotExistFails(self):
    with self.patch_call(self.call.device.build_version_sdk, return_value=29):
      with self.assertCalls((mock.call.os.path.exists(TEST_APEX_PATH), False)):
        with self.assertRaises(device_errors.CommandFailedError):
          self.device.InstallApex(DeviceUtilsInstallApexTest.mock_apex,
                                  retries=0)

  def testInstallApex_moduleNotInstalledFails(self):
    with self.patch_call(self.call.device.build_version_sdk, return_value=29):
      with self.assertCalls((mock.call.os.path.exists(TEST_APEX_PATH), True),
                            self.call.adb.Install(TEST_APEX_PATH),
                            self.call.device.Reboot(),
                            (self.call.device.IsSystemModuleInstalled(
                                TEST_PACKAGE, None), False)):
        with self.assertRaises(device_errors.CommandFailedError):
          self.device.InstallApex(DeviceUtilsInstallApexTest.mock_apex,
                                  retries=0)

  def testInstallApex_apexAlreadyStagedFails(self):
    with self.patch_call(self.call.device.build_version_sdk, return_value=29):
      with self.assertCalls((mock.call.os.path.exists(TEST_APEX_PATH), True), (
          self.call.adb.Install(TEST_APEX_PATH),
          self.AdbCommandError(
              output='Cannot stage multiple sessions without checkpoint support'
          ))):
        with self.assertRaises(device_errors.CommandFailedError):
          self.device.InstallApex(DeviceUtilsInstallApexTest.mock_apex,
                                  retries=0)

  def testInstallApex_deviceDoesntSupportApex(self):
    with self.patch_call(self.call.device.build_version_sdk, return_value=29):
      with self.assertCalls(
          (mock.call.os.path.exists(TEST_APEX_PATH), True),
          (self.call.adb.Install(TEST_APEX_PATH),
           self.AdbCommandError(
               output="device doesn't support the installation of APEX"))):
        with self.assertRaises(device_errors.CommandFailedError):
          self.device.InstallApex(DeviceUtilsInstallApexTest.mock_apex,
                                  retries=0)

  def testInstallApex_otherAdbErrorsChanneledThrough(self):
    with self.patch_call(self.call.device.build_version_sdk, return_value=29):
      with self.assertCalls((mock.call.os.path.exists(TEST_APEX_PATH), True),
                            (self.call.adb.Install(TEST_APEX_PATH),
                             self.AdbCommandError(output="An adb error"))):
        with self.assertRaises(device_errors.AdbCommandFailedError):
          self.device.InstallApex(DeviceUtilsInstallApexTest.mock_apex,
                                  retries=0)


class DeviceUtilsInstallSplitApkTest(DeviceUtilsTest):

  mock_apk = _MockApkHelper('base.apk', TEST_PACKAGE, ['p1'],
                            ['split1.apk', 'split2.apk'])

  def testInstallSplitApk_noPriorInstall(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (mock.call.devil.android.apk_helper.ToSplitHelper(
              'base.apk', ['split1.apk', 'split2.apk']),
           DeviceUtilsInstallSplitApkTest.mock_apk),
          (self.call.device._CheckSdkLevel(21)),
          (mock.call.os.path.exists('base.apk'), True),
          (mock.call.os.path.exists('split1.apk'), True),
          (mock.call.os.path.exists('split2.apk'), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          (self.call.adb.InstallMultiple(
              ['base.apk', 'split1.apk', 'split2.apk'],
              partial=None,
              reinstall=False,
              streaming=None,
              allow_downgrade=False,
              instant_app=False,
              force_queryable=False)),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.InstallSplitApk(
            'base.apk', ['split1.apk', 'split2.apk'], permissions=[], retries=0)

  def testInstallSplitApk_noStreaming(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='flounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (mock.call.devil.android.apk_helper.ToSplitHelper(
              'base.apk', ['split1.apk', 'split2.apk']),
           DeviceUtilsInstallSplitApkTest.mock_apk),
          (self.call.device._CheckSdkLevel(21)),
          (mock.call.os.path.exists('base.apk'), True),
          (mock.call.os.path.exists('split1.apk'), True),
          (mock.call.os.path.exists('split2.apk'), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          (self.call.adb.InstallMultiple(
              ['base.apk', 'split1.apk', 'split2.apk'],
              partial=None,
              reinstall=False,
              streaming=False,
              allow_downgrade=False,
              instant_app=False,
              force_queryable=False)),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.InstallSplitApk(
            'base.apk', ['split1.apk', 'split2.apk'], permissions=[], retries=0)

  def testInstallSplitApk_partialInstall(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (mock.call.devil.android.apk_helper.ToSplitHelper(
              DeviceUtilsInstallSplitApkTest.mock_apk,
              ['split1.apk', 'split2.apk']),
           DeviceUtilsInstallSplitApkTest.mock_apk),
          (self.call.device._CheckSdkLevel(21)),
          (mock.call.os.path.exists('base.apk'), True),
          (mock.call.os.path.exists('split1.apk'), True),
          (mock.call.os.path.exists('split2.apk'), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE),
           ['base-on-device.apk', 'split2-on-device.apk']),
          (self.call.device._ComputeStaleApks(
              TEST_PACKAGE, ['base.apk', 'split1.apk', 'split2.apk']),
           (['split2.apk'], None)),
          (self.call.adb.InstallMultiple(['split2.apk'],
                                         partial=TEST_PACKAGE,
                                         reinstall=True,
                                         streaming=None,
                                         allow_downgrade=False,
                                         instant_app=False,
                                         force_queryable=False)),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.InstallSplitApk(
            DeviceUtilsInstallSplitApkTest.mock_apk,
            ['split1.apk', 'split2.apk'],
            reinstall=True,
            permissions=[],
            retries=0)

  def testInstallSplitApk_downgrade(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (mock.call.devil.android.apk_helper.ToSplitHelper(
              DeviceUtilsInstallSplitApkTest.mock_apk,
              ['split1.apk', 'split2.apk']),
           DeviceUtilsInstallSplitApkTest.mock_apk),
          (self.call.device._CheckSdkLevel(21)),
          (mock.call.os.path.exists('base.apk'), True),
          (mock.call.os.path.exists('split1.apk'), True),
          (mock.call.os.path.exists('split2.apk'), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE),
           ['base-on-device.apk', 'split2-on-device.apk']),
          (self.call.device._ComputeStaleApks(
              TEST_PACKAGE, ['base.apk', 'split1.apk', 'split2.apk']),
           (['split2.apk'], None)),
          (self.call.adb.InstallMultiple(['split2.apk'],
                                         partial=TEST_PACKAGE,
                                         reinstall=True,
                                         streaming=None,
                                         allow_downgrade=True,
                                         instant_app=False,
                                         force_queryable=False)),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.InstallSplitApk(
            DeviceUtilsInstallSplitApkTest.mock_apk,
            ['split1.apk', 'split2.apk'],
            reinstall=True,
            permissions=[],
            retries=0,
            allow_downgrade=True)

  def testInstallSplitApk_missingSplit(self):
    with self.assertCalls(
        (mock.call.devil.android.apk_helper.ToSplitHelper(
            DeviceUtilsInstallSplitApkTest.mock_apk,
            ['split1.apk', 'split2.apk']),
          DeviceUtilsInstallSplitApkTest.mock_apk),
        (self.call.device._CheckSdkLevel(21)),
        (mock.call.os.path.exists('base.apk'), True),
        (mock.call.os.path.exists('split1.apk'), True),
        (mock.call.os.path.exists('split2.apk'), False)),\
        self.assertRaises(device_errors.CommandFailedError):
      self.device.InstallSplitApk(
          DeviceUtilsInstallSplitApkTest.mock_apk, ['split1.apk', 'split2.apk'],
          permissions=[],
          retries=0)

  def testInstallSplitApk_previouslyNonSplit(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (mock.call.devil.android.apk_helper.ToSplitHelper(
              DeviceUtilsInstallSplitApkTest.mock_apk,
              ['split1.apk', 'split2.apk']),
           DeviceUtilsInstallSplitApkTest.mock_apk),
          (self.call.device._CheckSdkLevel(21)),
          (mock.call.os.path.exists('base.apk'), True),
          (mock.call.os.path.exists('split1.apk'), True),
          (mock.call.os.path.exists('split2.apk'), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE),
           ['/fake/data/app/test.package.apk']),
          self.call.device.Uninstall(TEST_PACKAGE),
          (self.call.adb.InstallMultiple(
              ['base.apk', 'split1.apk', 'split2.apk'],
              partial=None,
              reinstall=False,
              streaming=None,
              allow_downgrade=False,
              instant_app=False,
              force_queryable=False)),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.InstallSplitApk(
            DeviceUtilsInstallSplitApkTest.mock_apk,
            ['split1.apk', 'split2.apk'],
            permissions=[],
            retries=0)

  def testInstallSplitApk_instantApp(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (mock.call.devil.android.apk_helper.ToSplitHelper(
              'base.apk', ['split1.apk', 'split2.apk']),
           DeviceUtilsInstallSplitApkTest.mock_apk),
          (self.call.device._CheckSdkLevel(21)),
          (mock.call.os.path.exists('base.apk'), True),
          (mock.call.os.path.exists('split1.apk'), True),
          (mock.call.os.path.exists('split2.apk'), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          (self.call.adb.InstallMultiple(
              ['base.apk', 'split1.apk', 'split2.apk'],
              partial=None,
              reinstall=False,
              streaming=None,
              allow_downgrade=False,
              instant_app=True,
              force_queryable=False)),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.InstallSplitApk('base.apk', ['split1.apk', 'split2.apk'],
                                    permissions=[],
                                    retries=0,
                                    instant_app=True)

  def testInstallSplitApk_forceQueryable(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='notflounder'), \
         self.patch_call(self.call.device.is_emulator, return_value=False):
      with self.assertCalls(
          (mock.call.devil.android.apk_helper.ToSplitHelper(
              'base.apk', ['split1.apk', 'split2.apk']),
           DeviceUtilsInstallSplitApkTest.mock_apk),
          (self.call.device._CheckSdkLevel(21)),
          (mock.call.os.path.exists('base.apk'), True),
          (mock.call.os.path.exists('split1.apk'), True),
          (mock.call.os.path.exists('split2.apk'), True),
          (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), []),
          (self.call.adb.InstallMultiple(
              ['base.apk', 'split1.apk', 'split2.apk'],
              partial=None,
              reinstall=False,
              streaming=None,
              allow_downgrade=False,
              instant_app=False,
              force_queryable=True)),
          (self.call.device.IsApplicationInstalled(TEST_PACKAGE, None), True)):
        self.device.InstallSplitApk('base.apk', ['split1.apk', 'split2.apk'],
                                    permissions=[],
                                    retries=0,
                                    force_queryable=True)


class DeviceUtilsUninstallTest(DeviceUtilsTest):
  def testUninstall_callsThrough(self):
    with self.assertCalls(
        (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE),
         ['/path.apk']), self.call.adb.Uninstall(TEST_PACKAGE, True)):
      self.device.Uninstall(TEST_PACKAGE, True)

  def testUninstall_noop(self):
    with self.assertCalls(
        (self.call.device._GetApplicationPathsInternal(TEST_PACKAGE), [])):
      self.device.Uninstall(TEST_PACKAGE, True)


class DeviceUtilsSuTest(DeviceUtilsTest):
  def testSu_preM(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP_MR1):
      self.assertEqual('su -c foo', self.device._Su('foo'))

  def testSu_mAndAbove(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.MARSHMALLOW):
      self.assertEqual('su 0 foo', self.device._Su('foo'))


class DeviceUtilsRunShellCommandTest(DeviceUtilsTest):
  def setUp(self):
    super(DeviceUtilsRunShellCommandTest, self).setUp()
    self.device.NeedsSU = mock.Mock(return_value=False)

  def testRunShellCommand_commandAsList(self):
    with self.assertCall(self.call.adb.Shell('pm list packages'), ''):
      self.device.RunShellCommand(['pm', 'list', 'packages'], check_return=True)

  def testRunShellCommand_commandAsListQuoted(self):
    with self.assertCall(self.call.adb.Shell("echo 'hello world' '$10'"), ''):
      self.device.RunShellCommand(['echo', 'hello world', '$10'],
                                  check_return=True)

  def testRunShellCommand_commandAsString(self):
    with self.assertCall(self.call.adb.Shell('echo "$VAR"'), ''):
      self.device.RunShellCommand('echo "$VAR"', shell=True, check_return=True)

  def testNewRunShellImpl_withEnv(self):
    with self.assertCall(
        self.call.adb.Shell('VAR=some_string echo "$VAR"'), ''):
      self.device.RunShellCommand(
          'echo "$VAR"',
          shell=True,
          check_return=True,
          env={'VAR': 'some_string'})

  def testNewRunShellImpl_withEnvQuoted(self):
    with self.assertCall(
        self.call.adb.Shell('PATH="$PATH:/other/path" run_this'), ''):
      self.device.RunShellCommand(['run_this'],
                                  check_return=True,
                                  env={'PATH': '$PATH:/other/path'})

  def testNewRunShellImpl_withEnv_failure(self):
    with self.assertRaises(KeyError):
      self.device.RunShellCommand(['some_cmd'],
                                  check_return=True,
                                  env={'INVALID NAME': 'value'})

  def testNewRunShellImpl_withCwd(self):
    with self.assertCall(self.call.adb.Shell('cd /some/test/path && ls'), ''):
      self.device.RunShellCommand(['ls'],
                                  check_return=True,
                                  cwd='/some/test/path')

  def testNewRunShellImpl_withCwdQuoted(self):
    with self.assertCall(
        self.call.adb.Shell("cd '/some test/path with/spaces' && ls"), ''):
      self.device.RunShellCommand(['ls'],
                                  check_return=True,
                                  cwd='/some test/path with/spaces')

  def testRunShellCommand_withHugeCmd(self):
    payload = 'hi! ' * 1024
    expected_cmd = "echo '%s'" % payload
    with self.assertCalls(
        (mock.call.devil.android.device_temp_file.DeviceTempFile(
            self.adb, suffix='.sh'), MockTempFile('/sdcard/temp-123.sh')),
        self.call.device._WriteFileWithPush('/sdcard/temp-123.sh',
                                            expected_cmd),
        (self.call.adb.Shell('sh /sdcard/temp-123.sh'), payload + '\n')):
      self.assertEqual([payload],
                       self.device.RunShellCommand(['echo', payload],
                                                   check_return=True))

  def testRunShellCommand_withHugeCmdAndSu(self):
    payload = 'hi! ' * 1024
    expected_cmd_without_su = """sh -c 'echo '"'"'%s'"'"''""" % payload
    expected_cmd = 'su -c %s' % expected_cmd_without_su
    with self.assertCalls(
        (self.call.device.NeedsSU(), True),
        (self.call.device._Su(expected_cmd_without_su), expected_cmd),
        (mock.call.devil.android.device_temp_file.DeviceTempFile(
            self.adb, suffix='.sh'), MockTempFile('/sdcard/temp-123.sh')),
        self.call.device._WriteFileWithPush('/sdcard/temp-123.sh',
                                            expected_cmd),
        (self.call.adb.Shell('sh /sdcard/temp-123.sh'), payload + '\n')):
      self.assertEqual([payload],
                       self.device.RunShellCommand(['echo', payload],
                                                   check_return=True,
                                                   as_root=True))

  def testRunShellCommand_withSu(self):
    expected_cmd_without_su = "sh -c 'setprop service.adb.root 0'"
    expected_cmd = 'su -c %s' % expected_cmd_without_su
    with self.assertCalls(
        (self.call.device.NeedsSU(), True),
        (self.call.device._Su(expected_cmd_without_su), expected_cmd),
        (self.call.adb.Shell(expected_cmd), '')):
      self.device.RunShellCommand(['setprop', 'service.adb.root', '0'],
                                  check_return=True,
                                  as_root=True)

  def testRunShellCommand_withRunAs(self):
    expected_cmd_without_run_as = "sh -c 'mkdir -p files'"
    expected_cmd = (
        'run-as org.devil.test_package %s' % expected_cmd_without_run_as)
    with self.assertCall(self.call.adb.Shell(expected_cmd), ''):
      self.device.RunShellCommand(['mkdir', '-p', 'files'],
                                  check_return=True,
                                  run_as='org.devil.test_package')

  def testRunShellCommand_withRunAsAndSu(self):
    expected_cmd_with_nothing = "sh -c 'mkdir -p files'"
    expected_cmd_with_run_as = (
        'run-as org.devil.test_package %s' % expected_cmd_with_nothing)
    expected_cmd_without_su = (
        'sh -c %s' % cmd_helper.SingleQuote(expected_cmd_with_run_as))
    expected_cmd = 'su -c %s' % expected_cmd_without_su
    with self.assertCalls(
        (self.call.device.NeedsSU(), True),
        (self.call.device._Su(expected_cmd_without_su), expected_cmd),
        (self.call.adb.Shell(expected_cmd), '')):
      self.device.RunShellCommand(['mkdir', '-p', 'files'],
                                  check_return=True,
                                  run_as='org.devil.test_package',
                                  as_root=True)

  def testRunShellCommand_manyLines(self):
    cmd = 'ls /some/path'
    with self.assertCall(self.call.adb.Shell(cmd), 'file1\nfile2\nfile3\n'):
      self.assertEqual(['file1', 'file2', 'file3'],
                       self.device.RunShellCommand(cmd.split(),
                                                   check_return=True))

  def testRunShellCommand_manyLinesRawOutput(self):
    cmd = 'ls /some/path'
    with self.assertCall(self.call.adb.Shell(cmd), '\rfile1\nfile2\r\nfile3\n'):
      self.assertEqual(
          '\rfile1\nfile2\r\nfile3\n',
          self.device.RunShellCommand(cmd.split(),
                                      check_return=True,
                                      raw_output=True))

  def testRunShellCommand_singleLine_success(self):
    cmd = 'echo $VALUE'
    with self.assertCall(self.call.adb.Shell(cmd), 'some value\n'):
      self.assertEqual(
          'some value',
          self.device.RunShellCommand(cmd,
                                      shell=True,
                                      check_return=True,
                                      single_line=True))

  def testRunShellCommand_singleLine_successEmptyLine(self):
    cmd = 'echo $VALUE'
    with self.assertCall(self.call.adb.Shell(cmd), '\n'):
      self.assertEqual(
          '',
          self.device.RunShellCommand(cmd,
                                      shell=True,
                                      check_return=True,
                                      single_line=True))

  def testRunShellCommand_singleLine_successWithoutEndLine(self):
    cmd = 'echo -n $VALUE'
    with self.assertCall(self.call.adb.Shell(cmd), 'some value'):
      self.assertEqual(
          'some value',
          self.device.RunShellCommand(cmd,
                                      shell=True,
                                      check_return=True,
                                      single_line=True))

  def testRunShellCommand_singleLine_successNoOutput(self):
    cmd = 'echo -n $VALUE'
    with self.assertCall(self.call.adb.Shell(cmd), ''):
      self.assertEqual(
          '',
          self.device.RunShellCommand(cmd,
                                      shell=True,
                                      check_return=True,
                                      single_line=True))

  def testRunShellCommand_singleLine_failTooManyLines(self):
    cmd = 'echo $VALUE'
    with self.assertCall(
        self.call.adb.Shell(cmd), 'some value\nanother value\n'):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.RunShellCommand(
            cmd, shell=True, check_return=True, single_line=True)

  def testRunShellCommand_checkReturn_success(self):
    cmd = 'echo $ANDROID_DATA'
    output = '/data\n'
    with self.assertCall(self.call.adb.Shell(cmd), output):
      self.assertEqual([output.rstrip()],
                       self.device.RunShellCommand(cmd,
                                                   shell=True,
                                                   check_return=True))

  def testRunShellCommand_checkReturn_failure(self):
    cmd = 'ls /root'
    output = 'opendir failed, Permission denied\n'
    with self.assertCall(self.call.adb.Shell(cmd), self.ShellError(output)):
      with self.assertRaises(device_errors.AdbCommandFailedError):
        self.device.RunShellCommand(cmd.split(), check_return=True)

  def testRunShellCommand_checkReturn_disabled(self):
    cmd = 'ls /root'
    output = 'opendir failed, Permission denied\n'
    with self.assertCall(self.call.adb.Shell(cmd), self.ShellError(output)):
      self.assertEqual([output.rstrip()],
                       self.device.RunShellCommand(cmd.split(),
                                                   check_return=False))

  def testRunShellCommand_largeOutput_enabled(self):
    cmd = 'echo $VALUE'
    temp_file = MockTempFile('/sdcard/temp-123')
    cmd_redirect = '( %s )>%s 2>&1' % (cmd, temp_file.name)
    with self.assertCalls(
        (mock.call.devil.android.device_temp_file.DeviceTempFile(
            self.adb), temp_file), (self.call.adb.Shell(cmd_redirect)),
        (self.call.device.ReadFile(
            temp_file.name, force_pull=True, encoding='utf8'), 'something')):
      self.assertEqual(['something'],
                       self.device.RunShellCommand(cmd,
                                                   shell=True,
                                                   large_output=True,
                                                   check_return=True))

  def testRunShellCommand_largeOutput_disabledNoTrigger(self):
    cmd = 'something'
    with self.assertCall(self.call.adb.Shell(cmd), self.ShellError('')):
      with self.assertRaises(device_errors.AdbCommandFailedError):
        self.device.RunShellCommand([cmd], check_return=True)

  def testRunShellCommand_largeOutput_disabledTrigger(self):
    cmd = 'echo $VALUE'
    temp_file = MockTempFile('/sdcard/temp-123')
    cmd_redirect = '( %s )>%s 2>&1' % (cmd, temp_file.name)
    with self.assertCalls(
        (self.call.adb.Shell(cmd), self.ShellError('', None)),
        (mock.call.devil.android.device_temp_file.DeviceTempFile(
            self.adb), temp_file), (self.call.adb.Shell(cmd_redirect)),
        (self.call.device.ReadFile(mock.ANY, force_pull=True,
                                   encoding='utf8'), 'something')):
      self.assertEqual(['something'],
                       self.device.RunShellCommand(cmd,
                                                   shell=True,
                                                   check_return=True))


class DeviceUtilsRunPipedShellCommandTest(DeviceUtilsTest):
  def testRunPipedShellCommand_success(self):
    with self.assertCall(
        self.call.device.RunShellCommand(
            'ps | grep foo; echo "PIPESTATUS: ${PIPESTATUS[@]}"',
            shell=True,
            check_return=True), ['This line contains foo', 'PIPESTATUS: 0 0']):
      self.assertEqual(['This line contains foo'],
                       self.device._RunPipedShellCommand('ps | grep foo'))

  def testRunPipedShellCommand_firstCommandFails(self):
    with self.assertCall(
        self.call.device.RunShellCommand(
            'ps | grep foo; echo "PIPESTATUS: ${PIPESTATUS[@]}"',
            shell=True,
            check_return=True), ['PIPESTATUS: 1 0']):
      with self.assertRaises(device_errors.AdbShellCommandFailedError) as ec:
        self.device._RunPipedShellCommand('ps | grep foo')
      self.assertEqual([1, 0], ec.exception.status)

  def testRunPipedShellCommand_secondCommandFails(self):
    with self.assertCall(
        self.call.device.RunShellCommand(
            'ps | grep foo; echo "PIPESTATUS: ${PIPESTATUS[@]}"',
            shell=True,
            check_return=True), ['PIPESTATUS: 0 1']):
      with self.assertRaises(device_errors.AdbShellCommandFailedError) as ec:
        self.device._RunPipedShellCommand('ps | grep foo')
      self.assertEqual([0, 1], ec.exception.status)

  def testRunPipedShellCommand_outputCutOff(self):
    with self.assertCall(
        self.call.device.RunShellCommand(
            'ps | grep foo; echo "PIPESTATUS: ${PIPESTATUS[@]}"',
            shell=True,
            check_return=True), ['foo.bar'] * 256 + ['foo.ba']):
      with self.assertRaises(device_errors.AdbShellCommandFailedError) as ec:
        self.device._RunPipedShellCommand('ps | grep foo')
      self.assertIs(None, ec.exception.status)


@mock.patch('time.sleep', mock.Mock())
class DeviceUtilsKillAllTest(DeviceUtilsTest):
  def testKillAll_noMatchingProcessesFailure(self):
    with self.assertCall(self.call.device.ListProcesses('test_process'), []):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.KillAll('test_process')

  def testKillAll_noMatchingProcessesQuiet(self):
    with self.assertCall(self.call.device.ListProcesses('test_process'), []):
      self.assertEqual(0, self.device.KillAll('test_process', quiet=True))

  def testKillAll_nonblocking(self):
    with self.assertCalls((self.call.device.ListProcesses('some.process'),
                           Processes(('some.process', 1234),
                                     ('some.process.thing', 5678))),
                          (self.call.adb.Shell('kill -9 1234 5678'), '')):
      self.assertEqual(2, self.device.KillAll('some.process', blocking=False))

  def testKillAll_blocking(self):
    with self.assertCalls(
        (self.call.device.ListProcesses('some.process'),
         Processes(('some.process', 1234), ('some.process.thing', 5678))),
        (self.call.adb.Shell('kill -9 1234 5678'), ''),
        (self.call.device.ListProcesses('some.process'),
         Processes(('some.process.thing', 5678))),
        (
            self.call.device.ListProcesses('some.process'),
            # Other instance with different pid.
            Processes(('some.process', 111)))):
      self.assertEqual(2, self.device.KillAll('some.process', blocking=True))

  def testKillAll_exactNonblocking(self):
    with self.assertCalls((self.call.device.ListProcesses('some.process'),
                           Processes(('some.process', 1234),
                                     ('some.process.thing', 5678))),
                          (self.call.adb.Shell('kill -9 1234'), '')):
      self.assertEqual(
          1, self.device.KillAll('some.process', exact=True, blocking=False))

  def testKillAll_exactBlocking(self):
    with self.assertCalls((self.call.device.ListProcesses('some.process'),
                           Processes(('some.process', 1234),
                                     ('some.process.thing', 5678))),
                          (self.call.adb.Shell('kill -9 1234'), ''),
                          (self.call.device.ListProcesses('some.process'),
                           Processes(('some.process', 1234),
                                     ('some.process.thing', 5678))),
                          (self.call.device.ListProcesses('some.process'),
                           Processes(('some.process.thing', 5678)))):
      self.assertEqual(
          1, self.device.KillAll('some.process', exact=True, blocking=True))

  def testKillAll_root(self):
    with self.assertCalls(
        (self.call.device.ListProcesses('some.process'),
         Processes(('some.process', 1234))), (self.call.device.NeedsSU(), True),
        (self.call.device._Su("sh -c 'kill -9 1234'"),
         "su -c sh -c 'kill -9 1234'"),
        (self.call.adb.Shell("su -c sh -c 'kill -9 1234'"), '')):
      self.assertEqual(1, self.device.KillAll('some.process', as_root=True))

  def testKillAll_sigterm(self):
    with self.assertCalls((self.call.device.ListProcesses('some.process'),
                           Processes(('some.process', 1234))),
                          (self.call.adb.Shell('kill -15 1234'), '')):
      self.assertEqual(
          1, self.device.KillAll('some.process', signum=device_signal.SIGTERM))

  def testKillAll_multipleInstances(self):
    with self.assertCalls((self.call.device.ListProcesses('some.process'),
                           Processes(('some.process', 1234),
                                     ('some.process', 4567))),
                          (self.call.adb.Shell('kill -15 1234 4567'), '')):
      self.assertEqual(
          2, self.device.KillAll('some.process', signum=device_signal.SIGTERM))


class DeviceUtilsStartActivityTest(DeviceUtilsTest):
  def testStartActivity_actionOnly(self):
    test_intent = intent.Intent(action='android.intent.action.VIEW')
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-a android.intent.action.VIEW'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(test_intent)

  def testStartActivity_actionOnly_currentUser(self):
    test_intent = intent.Intent(action='android.intent.action.VIEW')
    with self.patch_call(self.call.device.target_user, return_value=11):
      with self.assertCalls(
          (self.call.adb.Shell('am start --user 11 '
                               '-a android.intent.action.VIEW'),
           'Starting: Intent { act=android.intent.action.VIEW }')):
        self.device.StartActivity(test_intent)

  def testStartActivity_success(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main')
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-a android.intent.action.VIEW '
                            '-n test.package/.Main'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(test_intent)

  def testStartActivity_failure(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main')
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-a android.intent.action.VIEW '
                            '-n test.package/.Main'),
        'Error: Failed to start test activity'):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.StartActivity(test_intent)

  def testStartActivity_blocking(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main')
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-W '
                            '-a android.intent.action.VIEW '
                            '-n test.package/.Main'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(test_intent, blocking=True)

  def testStartActivity_withCategory(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main',
        category='android.intent.category.HOME')
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-a android.intent.action.VIEW '
                            '-c android.intent.category.HOME '
                            '-n test.package/.Main'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(test_intent)

  def testStartActivity_withMultipleCategories(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main',
        category=[
            'android.intent.category.HOME', 'android.intent.category.BROWSABLE'
        ])
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-a android.intent.action.VIEW '
                            '-c android.intent.category.HOME '
                            '-c android.intent.category.BROWSABLE '
                            '-n test.package/.Main'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(test_intent)

  def testStartActivity_withData(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main',
        data='http://www.google.com/')
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-a android.intent.action.VIEW '
                            '-d http://www.google.com/ '
                            '-n test.package/.Main'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(test_intent)

  def testStartActivity_withStringExtra(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main',
        extras={'foo': 'test'})
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-a android.intent.action.VIEW '
                            '-n test.package/.Main '
                            '--es foo test'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(test_intent)

  def testStartActivity_withBoolExtra(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main',
        extras={'foo': True})
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-a android.intent.action.VIEW '
                            '-n test.package/.Main '
                            '--ez foo True'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(test_intent)

  def testStartActivity_withIntExtra(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main',
        extras={'foo': 123})
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-a android.intent.action.VIEW '
                            '-n test.package/.Main '
                            '--ei foo 123'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(test_intent)

  def testStartActivity_withTraceFile(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main')
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '--start-profiler test_trace_file.out '
                            '-a android.intent.action.VIEW '
                            '-n test.package/.Main'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(
          test_intent, trace_file_name='test_trace_file.out')

  def testStartActivity_withForceStop(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main')
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-S '
                            '-a android.intent.action.VIEW '
                            '-n test.package/.Main'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(test_intent, force_stop=True)

  def testStartActivity_withFlags(self):
    test_intent = intent.Intent(
        action='android.intent.action.VIEW',
        package=TEST_PACKAGE,
        activity='.Main',
        flags=[
            intent.FLAG_ACTIVITY_NEW_TASK,
            intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED
        ])
    with self.assertCall(
        self.call.adb.Shell('am start '
                            '-a android.intent.action.VIEW '
                            '-n test.package/.Main '
                            '-f 0x10200000'),
        'Starting: Intent { act=android.intent.action.VIEW }'):
      self.device.StartActivity(test_intent)


class DeviceUtilsStartServiceTest(DeviceUtilsTest):
  def testStartService_success(self):
    test_intent = intent.Intent(
        action='android.intent.action.START',
        package=TEST_PACKAGE,
        activity='.Main')
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.NOUGAT):
      with self.assertCall(
          self.call.adb.Shell('am startservice '
                              '-a android.intent.action.START '
                              '-n test.package/.Main'),
          'Starting service: Intent { act=android.intent.action.START }'):
        self.device.StartService(test_intent)

  def testStartService_success_currentUser(self):
    test_intent = intent.Intent(
        action='android.intent.action.START',
        package=TEST_PACKAGE,
        activity='.Main')
    with self.patch_call(self.call.device.build_version_sdk,
                         return_value=version_codes.NOUGAT), \
         self.patch_call(self.call.device.target_user,
                         return_value=11):
      with self.assertCalls(
          (self.call.adb.Shell('am startservice --user 11 '
                               '-a android.intent.action.START '
                               '-n test.package/.Main'),
           'Starting service: Intent { act=android.intent.action.START }')):
        self.device.StartService(test_intent)

  def testStartService_failure(self):
    test_intent = intent.Intent(
        action='android.intent.action.START',
        package=TEST_PACKAGE,
        activity='.Main')
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.NOUGAT):
      with self.assertCall(
          self.call.adb.Shell('am startservice '
                              '-a android.intent.action.START '
                              '-n test.package/.Main'),
          'Error: Failed to start test service'):
        with self.assertRaises(device_errors.CommandFailedError):
          self.device.StartService(test_intent)

  def testStartService_onOreo(self):
    test_intent = intent.Intent(
        action='android.intent.action.START',
        package=TEST_PACKAGE,
        activity='.Main')
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.OREO):
      with self.assertCall(
          self.call.adb.Shell('am start-service '
                              '-a android.intent.action.START '
                              '-n test.package/.Main'),
          'Starting service: Intent { act=android.intent.action.START }'):
        self.device.StartService(test_intent)


class DeviceUtilsStartInstrumentationTest(DeviceUtilsTest):
  def testStartInstrumentation_nothing(self):
    with self.assertCalls(
        self.call.device.RunShellCommand(
            'p=test.package;am instrument "$p"/.TestInstrumentation',
            shell=True,
            check_return=True,
            large_output=True)):
      self.device.StartInstrumentation(
          'test.package/.TestInstrumentation',
          finish=False,
          raw=False,
          extras=None)

  def testStartInstrumentation_finish(self):
    with self.assertCalls((self.call.device.RunShellCommand(
        'p=test.package;am instrument -w "$p"/.TestInstrumentation',
        shell=True,
        check_return=True,
        large_output=True), ['OK (1 test)'])):
      output = self.device.StartInstrumentation(
          'test.package/.TestInstrumentation',
          finish=True,
          raw=False,
          extras=None)
      self.assertEqual(['OK (1 test)'], output)

  def testStartInstrumentation_raw(self):
    with self.assertCalls(
        self.call.device.RunShellCommand(
            'p=test.package;am instrument -r "$p"/.TestInstrumentation',
            shell=True,
            check_return=True,
            large_output=True)):
      self.device.StartInstrumentation(
          'test.package/.TestInstrumentation',
          finish=False,
          raw=True,
          extras=None)

  def testStartInstrumentation_extras(self):
    with self.assertCalls(
        self.call.device.RunShellCommand(
            'p=test.package;am instrument -e "$p".foo Foo -e bar \'Val \'"$p" '
            '"$p"/.TestInstrumentation',
            shell=True,
            check_return=True,
            large_output=True)):
      self.device.StartInstrumentation(
          'test.package/.TestInstrumentation',
          finish=False,
          raw=False,
          extras={
              'test.package.foo': 'Foo',
              'bar': 'Val test.package'
          })

  def testStartInstrumentation_currentUser(self):
    cmd = 'p=test.package;am instrument --user 11 "$p"/.TestInstrumentation'
    with self.patch_call(self.call.device.target_user, return_value=11):
      with self.assertCalls(
          self.call.device.RunShellCommand(cmd,
                                           shell=True,
                                           check_return=True,
                                           large_output=True)):
        self.device.StartInstrumentation('test.package/.TestInstrumentation',
                                         finish=False,
                                         raw=False,
                                         extras=None)


class DeviceUtilsBroadcastIntentTest(DeviceUtilsTest):
  def testBroadcastIntent_noExtras(self):
    test_intent = intent.Intent(action='test.package.with.an.INTENT')
    with self.assertCall(
        self.call.adb.Shell('am broadcast -a test.package.with.an.INTENT'),
        'Broadcasting: Intent { act=test.package.with.an.INTENT } '):
      self.device.BroadcastIntent(test_intent)

  def testBroadcastIntent_withExtra(self):
    test_intent = intent.Intent(
        action='test.package.with.an.INTENT', extras={'foo': 'bar value'})
    with self.assertCall(
        self.call.adb.Shell(
            "am broadcast -a test.package.with.an.INTENT --es foo 'bar value'"),
        'Broadcasting: Intent { act=test.package.with.an.INTENT } '):
      self.device.BroadcastIntent(test_intent)

  def testBroadcastIntent_withExtra_noValue(self):
    test_intent = intent.Intent(
        action='test.package.with.an.INTENT', extras={'foo': None})
    with self.assertCall(
        self.call.adb.Shell(
            'am broadcast -a test.package.with.an.INTENT --esn foo'),
        'Broadcasting: Intent { act=test.package.with.an.INTENT } '):
      self.device.BroadcastIntent(test_intent)


class DeviceUtilGetCurrentUserTest(DeviceUtilsTest):
  def testGetCurrentUser_Nougat(self):
    user_id = 10
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '24'),
        (self.call.adb.Shell('am get-current-user'),
         'warning msg\n' + str(user_id))):
      self.assertEqual(user_id, self.device.GetCurrentUser())

  def testGetCurrentUser_PreNougat_SingleUser(self):
    user_id = 0
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '23'),
        (self.call.device._GetDumpsysOutput(
            ['activity'], 'mUserLru:'), ['  mUserLru: [%s]' % user_id])):
      self.assertEqual(user_id, self.device.GetCurrentUser())

  def testGetCurrentUser_PreNougat_MultipleUsers(self):
    user_id = 10
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '23'),
        (self.call.device._GetDumpsysOutput(
            ['activity'], 'mUserLru:'), ['  mUserLru: [0, %s]' % user_id])):
      self.assertEqual(user_id, self.device.GetCurrentUser())

  def testGetCurrentUser_PreNougat_Fails(self):
    user_id = 0
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '23'),
        (self.call.device._GetDumpsysOutput(['activity'], 'mUserLru:'), [])):
      with self.assertRaises(device_errors.CommandFailedError):
        self.assertEqual(user_id, self.device.GetCurrentUser())


class DeviceUtilsListUsersTest(DeviceUtilsTest):
  def testListUsers(self):
    list_users_output = [
        'Users:',
        '    UserInfo{0:Driver:c13} running',
        '    UserInfo{10:aa:bb:410}',
        '    UserInfo{11:Driver:412} running',
        '    UserInfo{14:guest3:400} (Secondary User, not-initialized)',
    ]
    expected_users = [
        {'id': 0, 'name': 'Driver', 'flags': 0xc13},
        {'id': 10, 'name': 'aa:bb', 'flags': 0x410},
        {'id': 11, 'name': 'Driver', 'flags': 0x412},
        {'id': 14, 'name': 'guest3', 'flags': 0x400},
    ]
    with self.assertCall(
        self.call.device.RunShellCommand(['pm', 'list', 'users'],
                                         check_return=True), list_users_output):
      self.assertEqual(expected_users, self.device.ListUsers())


class DeviceUtilsGetMainUserTest(DeviceUtilsTest):
  def testGetMainUser_with_flag_main_hsum(self):
    with self.assertCall(self.call.device.ListUsers(), [
        {'id': 0, 'name': 'Driver', 'flags': 0x813},
        {'id': 11, 'name': 'Driver', 'flags': 0x4412},
    ]):
      self.assertEqual(11, self.device.GetMainUser())

  def testGetMainUser_with_flag_main_non_hsum(self):
    with self.assertCall(self.call.device.ListUsers(), [
        {'id': 0, 'name': 'Driver', 'flags': 0x4c13},
        {'id': 11, 'name': 'Driver', 'flags': 0x412},
    ]):
      self.assertEqual(0, self.device.GetMainUser())

  def testGetMainUser_with_fallback_hsum(self):
    with self.assertCall(self.call.device.ListUsers(), [
        {'id': 0, 'name': 'Driver', 'flags': 0x813},
        {'id': 11, 'name': 'Driver', 'flags': 0x412},
    ]):
      self.assertEqual(11, self.device.GetMainUser())

  def testGetMainUser_with_fallback_non_hsum(self):
    with self.assertCall(self.call.device.ListUsers(), [
        {'id': 0, 'name': 'Driver', 'flags': 0xc13},
        {'id': 11, 'name': 'Driver', 'flags': 0x412},
    ]):
      self.assertEqual(0, self.device.GetMainUser())

  def testGetMainUser_failed(self):
    with self.assertCall(self.call.device.ListUsers(), [
        {'id': 0, 'name': 'Driver', 'flags': 0x813},
        {'id': 11, 'name': 'Driver', 'flags': 0x410},
    ]):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.GetMainUser()


class DeviceUtilSwitchUserTest(DeviceUtilsTest):
  def testSwitchUser(self):
    user_id = 10
    with self.assertCall(self.call.adb.Shell('am switch-user %s' % user_id),
                         ''):
      self.device.SwitchUser(user_id)


class DeviceUtilsGoHomeTest(DeviceUtilsTest):
  def testGoHome_popupsExist(self):
    with self.assertCalls(
        (self.call.device.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                          check_return=True,
                                          large_output=True), []),
        (self.call.device.RunShellCommand([
            'am', 'start', '-W', '-a', 'android.intent.action.MAIN', '-c',
            'android.intent.category.HOME'
        ],
                                          check_return=True),
         'Starting: Intent { act=android.intent.action.MAIN }\r\n'
         ''),
        (self.call.device.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                          check_return=True,
                                          large_output=True), []),
        (self.call.device.RunShellCommand(['input', 'keyevent', '66'],
                                          check_return=True)),
        (self.call.device.RunShellCommand(['input', 'keyevent', '4'],
                                          check_return=True)),
        (self.call.device.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                          check_return=True,
                                          large_output=True),
         ['mResumedActivity Launcher'])):
      self.device.GoHome()

  def testGoHome_willRetry(self):
    with self.assertCalls(
        (self.call.device.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                          check_return=True,
                                          large_output=True), []),
        (self.call.device.RunShellCommand([
            'am', 'start', '-W', '-a', 'android.intent.action.MAIN', '-c',
            'android.intent.category.HOME'
        ],
                                          check_return=True),
         'Starting: Intent { act=android.intent.action.MAIN }\r\n'
         ''),
        (self.call.device.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                          check_return=True,
                                          large_output=True), []),
        (self.call.device.RunShellCommand(
            ['input', 'keyevent', '66'],
            check_return=True,
        )), (self.call.device.RunShellCommand(['input', 'keyevent', '4'],
                                              check_return=True)),
        (self.call.device.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                          check_return=True,
                                          large_output=True), []),
        (self.call.device.RunShellCommand(['input', 'keyevent', '66'],
                                          check_return=True)),
        (self.call.device.RunShellCommand(['input', 'keyevent', '4'],
                                          check_return=True)),
        (self.call.device.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                          check_return=True,
                                          large_output=True),
         self.TimeoutError())):
      with self.assertRaises(device_errors.CommandTimeoutError):
        self.device.GoHome()

  def testGoHome_alreadyFocused(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                         check_return=True,
                                         large_output=True),
        ['mResumedActivity Launcher']):
      self.device.GoHome()

  def testGoHome_alreadyFocusedAlternateCase(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                         check_return=True,
                                         large_output=True),
        [' mResumedActivity .launcher/.']):
      self.device.GoHome()

  def testGoHome_alreadyFocusedWithTopResumedActivity(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                         check_return=True,
                                         large_output=True),
        ['topResumedActivity=LauncherActivity']):
      self.device.GoHome()

  def testGoHome_obtainsFocusAfterGoingHome(self):
    with self.assertCalls(
        (self.call.device.RunShellCommand(['dumpsys', 'activity', 'activities'],
                                          check_return=True,
                                          large_output=True), []),
        (self.call.device.RunShellCommand([
            'am', 'start', '-W', '-a', 'android.intent.action.MAIN', '-c',
            'android.intent.category.HOME'
        ],
                                          check_return=True),
         'Starting: Intent { act=android.intent.action.MAIN }\r\n'
         ''), (self.call.device.RunShellCommand(
             ['dumpsys', 'activity', 'activities'],
             check_return=True,
             large_output=True), ['mResumedActivity Launcher'])):
      self.device.GoHome()


class DeviceUtilsUnlockTest(DeviceUtilsTest):
  def testUnlock_AlreadyUnlocked(self):
    with self.assertCalls(
        (self.call.device.SendKeyEvent(keyevent.KEYCODE_WAKEUP), None),
        (self.call.device.RunShellCommand(['dumpsys', 'nfc'
                                           ]), ["mScreenState=ON_UNLOCKED"])):
      self.device.Unlock()

  def testUnlock_emulatorAlwaysUnlocked(self):
    with self.assertCalls(
        (self.call.device.SendKeyEvent(keyevent.KEYCODE_WAKEUP), None),
        (self.call.device.RunShellCommand(['dumpsys', 'nfc'
                                           ]), ["Can't find service: nfc"])):
      self.device.Unlock()

  def testUnlock_lockedWithoutPasscode(self):
    with self.assertCalls(
        (self.call.device.SendKeyEvent(keyevent.KEYCODE_WAKEUP), None),
        (self.call.device.RunShellCommand(['dumpsys', 'nfc'
                                           ]), ["mScreenState=ON_LOCKED"]),
        (self.call.device.SendKeyEvent(keyevent.KEYCODE_MENU), None),
        (self.call.device.RunShellCommand(['dumpsys', 'nfc']), [])):
      self.device.Unlock()

  def testUnlock_lockedRequiresPasscode(self):
    with self.assertCalls(
        (self.call.device.SendKeyEvent(keyevent.KEYCODE_WAKEUP), None),
        (self.call.device.RunShellCommand(['dumpsys', 'nfc'
                                           ]), ["mScreenState=ON_LOCKED"]),
        (self.call.device.SendKeyEvent(keyevent.KEYCODE_MENU), None),
        (self.call.device.RunShellCommand(['dumpsys', 'nfc'
                                           ]), ["mScreenState=ON_LOCKED"])):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.Unlock()


class DeviceUtilsForceStopTest(DeviceUtilsTest):
  def testForceStop(self):
    with self.assertCalls(
        (self.call.device.GetApplicationPids(TEST_PACKAGE), [1111]),
        (self.call.device.RunShellCommand(['am', 'force-stop', TEST_PACKAGE],
                                          check_return=True), ['Success'])):
      self.device.ForceStop(TEST_PACKAGE)

  def testForceStop_NoProcessFound(self):
    with self.assertCall(self.call.device.GetApplicationPids(TEST_PACKAGE), []):
      self.device.ForceStop(TEST_PACKAGE)


class DeviceUtilsClearApplicationStateTest(DeviceUtilsTest):
  def testClearApplicationState_setPermissions(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '17'),
        (self.call.device._GetApplicationPathsInternal('this.package.exists'),
         ['/data/app/this.package.exists.apk']),
        (self.call.device.RunShellCommand(
            ['pm', 'clear', 'this.package.exists'], check_return=True),
         ['Success']),
        (self.call.device.GrantPermissions('this.package.exists', ['p1']), [])):
      self.device.ClearApplicationState(
          'this.package.exists', permissions=['p1'])

  def testClearApplicationState_setPermissions_currentUser(self):
    with self.patch_call(self.call.device.target_user, return_value=11):
      with self.assertCalls(
          (self.call.device.GetProp('ro.build.version.sdk', cache=True), '17'),
          (self.call.device._GetApplicationPathsInternal('this.package.exists'),
           ['/data/app/this.package.exists.apk']),
          (self.call.device.RunShellCommand(
              ['pm', 'clear', '--user', '11', 'this.package.exists'],
              check_return=True), ['Success']),
          (self.call.device.GrantPermissions('this.package.exists',
                                             ['p1']), [])):
        self.device.ClearApplicationState('this.package.exists',
                                          permissions=['p1'])

  def testClearApplicationState_packageDoesntExist(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '11'),
        (self.call.device._GetApplicationPathsInternal('does.not.exist'), [])):
      self.device.ClearApplicationState('does.not.exist')

  def testClearApplicationState_packageDoesntExistOnAndroidJBMR2OrAbove(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '18'),
        (self.call.device.RunShellCommand(
            ['pm', 'clear', 'this.package.does.not.exist'], check_return=True),
         ['Failed'])):
      self.device.ClearApplicationState('this.package.does.not.exist')

  def testClearApplicationState_packageExists(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '17'),
        (self.call.device._GetApplicationPathsInternal('this.package.exists'),
         ['/data/app/this.package.exists.apk']),
        (self.call.device.RunShellCommand(
            ['pm', 'clear', 'this.package.exists'], check_return=True),
         ['Success'])):
      self.device.ClearApplicationState('this.package.exists')

  def testClearApplicationState_packageExistsOnAndroidJBMR2OrAbove(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '18'),
        (self.call.device.RunShellCommand(
            ['pm', 'clear', 'this.package.exists'], check_return=True),
         ['Success'])):
      self.device.ClearApplicationState('this.package.exists')

  def testClearApplicationState_waitForAsynchronousIntent(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '18'),
        (self.call.device.RunShellCommand(
            ['pm', 'clear', 'this.package.exists'],
            check_return=True), ['Success']), (self.call.device.RunShellCommand(
                [
                    'content', 'call', '--uri', 'content://media/external/file',
                    '--method', 'wait_for_idle'
                ],
                check_return=True), ['Result: null'])):
      self.device.ClearApplicationState('this.package.exists',
                                        wait_for_asynchronous_intent=True)


class DeviceUtilsSendKeyEventTest(DeviceUtilsTest):
  def testSendKeyEvent(self):
    with self.assertCall(self.call.adb.Shell('input keyevent 66'), ''):
      self.device.SendKeyEvent(66)


class DeviceUtilsPushChangedFilesIndividuallyTest(DeviceUtilsTest):
  def testPushChangedFilesIndividually_empty(self):
    test_files = []
    with self.assertCalls():
      self.device._PushChangedFilesIndividually(test_files)

  def testPushChangedFilesIndividually_single(self):
    test_files = [('/test/host/path', '/test/device/path')]
    with self.assertCalls(self.call.adb.Push(*test_files[0])):
      self.device._PushChangedFilesIndividually(test_files)

  def testPushChangedFilesIndividually_multiple(self):
    test_files = [('/test/host/path/file1', '/test/device/path/file1'),
                  ('/test/host/path/file2', '/test/device/path/file2')]
    with self.assertCalls(
        self.call.adb.Push(*test_files[0]), self.call.adb.Push(*test_files[1])):
      self.device._PushChangedFilesIndividually(test_files)


class DeviceUtilsPushChangedFilesZippedTest(DeviceUtilsTest):
  def testPushChangedFilesZipped_noUnzipCommand(self):
    test_files = [('/test/host/path/file1', '/test/device/path/file1')]
    with self.assertCalls((self.call.device._MaybeInstallCommands(), False)):
      self.assertFalse(
          self.device._PushChangedFilesZipped(test_files, ['/test/dir']))

  def _testPushChangedFilesZipped_spec(self, test_files, test_dirs):
    @contextlib.contextmanager
    def mock_zip_temp_dir():
      yield '/test/temp/dir'

    expected_cmd = ''.join([
        '\n  /data/local/tmp/bin/unzip %s &&',
        ' (for dir in %s\n  do\n    chmod -R 777 "$dir" || exit 1\n',
        '  done)\n'
    ]) % ('/sdcard/foo123.zip', ' '.join(
        cmd_helper.SingleQuote(d) for d in test_dirs))
    with self.assertCalls(
        (self.call.device._MaybeInstallCommands(), True),
        (mock.call.py_utils.tempfile_ext.NamedTemporaryDirectory(),
         mock_zip_temp_dir), (mock.call.devil.utils.zip_utils.WriteZipFile(
             '/test/temp/dir/tmp.zip', test_files)),
        (mock.call.os.path.getsize('/test/temp/dir/tmp.zip'), 123),
        (self.call.device.NeedsSU(), True),
        (mock.call.devil.android.device_temp_file.DeviceTempFile(
            self.adb, suffix='.zip'), MockTempFile('/sdcard/foo123.zip')),
        self.call.adb.Push('/test/temp/dir/tmp.zip', '/sdcard/foo123.zip'),
        (mock.call.devil.android.device_temp_file.DeviceTempFile(
            self.adb, suffix='.sh'), MockTempFile('/sdcard/temp-123.sh')),
        self.call.device.WriteFile('/sdcard/temp-123.sh', expected_cmd),
        (self.call.device.RunShellCommand(['source', '/sdcard/temp-123.sh'],
                                          check_return=True,
                                          as_root=True))):
      self.assertTrue(
          self.device._PushChangedFilesZipped(test_files, test_dirs))

  def testPushChangedFilesZipped_single(self):
    self._testPushChangedFilesZipped_spec(
        [('/test/host/path/file1', '/test/device/path/file1')],
        ['/test/dir1'])

  def testPushChangedFilesZipped_multiple(self):
    self._testPushChangedFilesZipped_spec(
        [('/test/host/path/file1', '/test/device/path/file1'),
         ('/test/host/path/file2', '/test/device/path/file2')],
        ['/test/dir1', '/test/dir2', '/test/dir with space'])


class DeviceUtilsPathExistsTest(DeviceUtilsTest):
  def testPathExists_pathExists(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['test', '-e', '/path/file exists'],
                                         as_root=False,
                                         check_return=True,
                                         timeout=10,
                                         retries=0), []):
      self.assertTrue(self.device.PathExists('/path/file exists'))

  def testPathExists_multiplePathExists(self):
    with self.assertCall(
        self.call.device.RunShellCommand(
            ['test', '-e', '/path 1', '-a', '-e', '/path2'],
            as_root=False,
            check_return=True,
            timeout=10,
            retries=0), []):
      self.assertTrue(self.device.PathExists(('/path 1', '/path2')))

  def testPathExists_pathDoesntExist(self):
    with self.assertCall(
        self.call.device.RunShellCommand(
            ['test', '-e', '/path/file.not.exists'],
            as_root=False,
            check_return=True,
            timeout=10,
            retries=0), self.ShellError()):
      self.assertFalse(self.device.PathExists('/path/file.not.exists'))

  def testPathExists_asRoot(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['test', '-e', '/root/path/exists'],
                                         as_root=True,
                                         check_return=True,
                                         timeout=10,
                                         retries=0), self.ShellError()):
      self.assertFalse(
          self.device.PathExists('/root/path/exists', as_root=True))

  def testFileExists_pathDoesntExist(self):
    with self.assertCall(
        self.call.device.RunShellCommand(
            ['test', '-e', '/path/file.not.exists'],
            as_root=False,
            check_return=True,
            timeout=10,
            retries=0), self.ShellError()):
      self.assertFalse(self.device.FileExists('/path/file.not.exists'))


class DeviceUtilsRemovePathTest(DeviceUtilsTest):
  def testRemovePath_regular(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['rm', 'some file'],
                                         as_root=False,
                                         check_return=True), []):
      self.device.RemovePath('some file')

  def testRemovePath_withForce(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['rm', '-f', 'some file'],
                                         as_root=False,
                                         check_return=True), []):
      self.device.RemovePath('some file', force=True)

  def testRemovePath_recursively(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['rm', '-r', '/remove/this/dir'],
                                         as_root=False,
                                         check_return=True), []):
      self.device.RemovePath('/remove/this/dir', recursive=True)

  def testRemovePath_withRoot(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['rm', 'some file'],
                                         as_root=True,
                                         check_return=True), []):
      self.device.RemovePath('some file', as_root=True)

  def testRemovePath_manyPaths(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['rm', 'eeny', 'meeny', 'miny', 'moe'],
                                         as_root=False,
                                         check_return=True), []):
      self.device.RemovePath(['eeny', 'meeny', 'miny', 'moe'])


class DeviceUtilsPullFileTest(DeviceUtilsTest):
  def testPullFile_existsOnDevice(self):
    with mock.patch('os.path.exists', return_value=True):
      with self.assertCall(
          self.call.adb.Pull('/data/app/test.file.exists',
                             '/test/file/host/path')):
        self.device.PullFile('/data/app/test.file.exists',
                             '/test/file/host/path')

  def testPullFile_doesntExistOnDevice(self):
    with mock.patch('os.path.exists', return_value=True):
      with self.assertCall(
          self.call.adb.Pull('/data/app/test.file.does.not.exist',
                             '/test/file/host/path'),
          self.CommandError('remote object does not exist')):
        with self.assertRaises(device_errors.CommandFailedError):
          self.device.PullFile('/data/app/test.file.does.not.exist',
                               '/test/file/host/path')

  def testPullFile_asRoot(self):
    with mock.patch('os.path.exists', return_value=True):
      with self.assertCalls(
          (self.call.device.NeedsSU(), True), (self.call.device.PathExists(
              '/this/file/can.be.read.with.su', as_root=True), True),
          (mock.call.devil.android.device_temp_file.DeviceTempFile(self.adb),
           MockTempFile('/sdcard/tmp/on.device')),
          self.call.device.RunShellCommand(
              'SRC=/this/file/can.be.read.with.su DEST=/sdcard/tmp/on.device;'
              'cp "$SRC" "$DEST" && chmod 666 "$DEST"',
              shell=True,
              as_root=True,
              check_return=True), (self.call.adb.Pull('/sdcard/tmp/on.device',
                                                      '/test/file/host/path'))):
        self.device.PullFile(
            '/this/file/can.be.read.with.su',
            '/test/file/host/path',
            as_root=True)

  def testPullFile_asRootDoesntExistOnDevice(self):
    with mock.patch('os.path.exists', return_value=True):
      with self.assertCalls(
          (self.call.device.NeedsSU(), True), (self.call.device.PathExists(
              '/data/app/test.file.does.not.exist', as_root=True), False)):
        with self.assertRaises(device_errors.CommandFailedError):
          self.device.PullFile(
              '/data/app/test.file.does.not.exist',
              '/test/file/host/path',
              as_root=True)


class DeviceUtilsReadFileTest(DeviceUtilsTest):
  def testReadFileWithPull_success(self):
    tmp_host_dir = '/tmp/dir/on.host/'
    tmp_host = MockTempFile('/tmp/dir/on.host/tmp_ReadFileWithPull')
    tmp_host.file.read.return_value = 'some interesting contents'
    with self.assertCalls(
        (mock.call.tempfile.mkdtemp(), tmp_host_dir),
        (self.call.adb.Pull('/path/to/device/file', mock.ANY)),
        (mock.call.__builtin__.open(mock.ANY, 'rb'), tmp_host) if six.PY2 else \
            (mock.call.builtins.open(mock.ANY, 'rb'), tmp_host),
        (mock.call.os.path.exists(tmp_host_dir), True),
        (mock.call.shutil.rmtree(tmp_host_dir), None)):
      self.assertEqual('some interesting contents',
                       self.device._ReadFileWithPull('/path/to/device/file'))
    tmp_host.file.read.assert_called_once_with()

  def _check_ReadFileWithEncodingErrors(self, encoding, errors):
    tmp_host_dir = '/tmp/dir/on.host/'
    tmp_host = MockTempFile('/tmp/dir/on.host/tmp_ReadFileWithEncodingErrors')
    file_content = b'file with all ' + bytes(bytearray(range(256))) + b' bytes'
    if six.PY2 or encoding is None:
      expected_content = file_content
    else:
      expected_content = file_content.decode(encoding, errors)
      self.assertNotEqual(file_content, expected_content)
    tmp_host.file.read.return_value = file_content
    with self.assertCalls(
        (mock.call.tempfile.mkdtemp(), tmp_host_dir),
        (self.call.adb.Pull('/path/to/device/file', mock.ANY)),
        (mock.call.__builtin__.open(mock.ANY, 'rb'), tmp_host) if six.PY2 else \
            (mock.call.builtins.open(mock.ANY, 'rb'), tmp_host),
        (mock.call.os.path.exists(tmp_host_dir), True),
        (mock.call.shutil.rmtree(tmp_host_dir), None)):
      self.assertEqual(expected_content,
                       self.device._ReadFileWithPull('/path/to/device/file',
                                                     encoding, errors))
    tmp_host.file.read.assert_called_once_with()

  def testReadFile_AsBytes(self):
    self._check_ReadFileWithEncodingErrors(None, 'replace')

  def testReadFile_NotUtf8_Replace(self):
    self._check_ReadFileWithEncodingErrors('utf8', 'replace')

  def testReadFile_NotUtf8_Ignore(self):
    self._check_ReadFileWithEncodingErrors('utf8', 'ignore')

  def testReadFile_NotCp1251_Replace(self):
    self._check_ReadFileWithEncodingErrors('cp1251', 'replace')

  def testReadFile_NotCp1251_Ignore(self):
    self._check_ReadFileWithEncodingErrors('cp1251', 'ignore')

  def testReadFileWithPull_rejected(self):
    tmp_host_dir = '/tmp/dir/on.host/'
    with self.assertCalls((mock.call.tempfile.mkdtemp(), tmp_host_dir),
                          (self.call.adb.Pull('/path/to/device/file', mock.ANY),
                           self.CommandError()),
                          (mock.call.os.path.exists(tmp_host_dir), True),
                          (mock.call.shutil.rmtree(tmp_host_dir), None)):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device._ReadFileWithPull('/path/to/device/file')

  def testReadFile_withSU_zeroSize(self):
    with self.assertCalls(
        (self.call.device.NeedsSU(), True),
        (self.call.device.FileSize(
            '/this/file/has/zero/size', as_root=True), 0),
        (mock.call.devil.android.device_temp_file.DeviceTempFile(self.adb),
         MockTempFile('/sdcard/tmp/on.device')),
        self.call.device.RunShellCommand(
            'SRC=/this/file/has/zero/size DEST=/sdcard/tmp/on.device;'
            'cp "$SRC" "$DEST" && chmod 666 "$DEST"',
            shell=True,
            as_root=True,
            check_return=True),
        (self.call.device._ReadFileWithPull('/sdcard/tmp/on.device',
                                            'utf8', 'replace'),
         'but it has contents\n')):
      self.assertEqual('but it has contents\n',
                       self.device.ReadFile('/this/file/has/zero/size',
                       as_root=True))

  def testReadFile_withSU(self):
    with self.assertCalls(
        (self.call.device.NeedsSU(), True),
        (self.call.device.FileSize(
            '/this/file/can.be.read.with.su', as_root=True), 256),
        (self.call.device.RunShellCommand(
            ['cat', '/this/file/can.be.read.with.su'],
            as_root=True,
            check_return=True), ['this is a test file', 'read with su'])):
      self.assertEqual(
          'this is a test file\nread with su\n',
          self.device.ReadFile('/this/file/can.be.read.with.su', as_root=True))

  def testReadFile_withSU_doesNotExist(self):
    with self.assertCalls(
        (self.call.device.NeedsSU(), True),
        (self.call.device.FileSize('/this/file/does.not.exist', as_root=True),
         self.CommandError('File does not exist'))):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.ReadFile('/this/file/does.not.exist', as_root=True)

  def testReadFile_withPull(self):
    contents = 'a' * 123456
    with self.assertCalls(
        (self.call.device._ReadFileWithPull('/read/this/big/test/file',
                                            'utf8', 'replace'),
         contents)):
      self.assertEqual(contents,
                       self.device.ReadFile('/read/this/big/test/file'))

  def testReadFile_withPullAndSU(self):
    contents = 'b' * 123456
    with self.assertCalls(
        (self.call.device.NeedsSU(), True),
        (self.call.device.FileSize(
            '/this/big/file/can.be.read.with.su', as_root=True), 123456),
        (mock.call.devil.android.device_temp_file.DeviceTempFile(self.adb),
         MockTempFile('/sdcard/tmp/on.device')),
        self.call.device.RunShellCommand(
            'SRC=/this/big/file/can.be.read.with.su DEST=/sdcard/tmp/on.device;'
            'cp "$SRC" "$DEST" && chmod 666 "$DEST"',
            shell=True,
            as_root=True,
            check_return=True),
        (self.call.device._ReadFileWithPull('/sdcard/tmp/on.device',
                                            'utf8', 'replace'),
         contents)):
      self.assertEqual(
          contents,
          self.device.ReadFile(
              '/this/big/file/can.be.read.with.su', as_root=True))

  def testReadFile_forcePull(self):
    contents = 'a' * 123456
    with self.assertCall(
        self.call.device._ReadFileWithPull('/read/this/big/test/file',
                                           'utf8', 'replace'),
        contents):
      self.assertEqual(
          contents,
          self.device.ReadFile('/read/this/big/test/file', force_pull=True))


class DeviceUtilsWriteFileTest(DeviceUtilsTest):
  def testWriteFileWithPush_success(self):
    tmp_host = MockTempFile('/tmp/file/on.host')
    contents = 'some interesting contents'
    with self.assertCalls(
        (mock.call.tempfile.NamedTemporaryFile(mode='w+'), tmp_host),
        self.call.adb.Push('/tmp/file/on.host', '/path/to/device/file')):
      self.device._WriteFileWithPush('/path/to/device/file', contents)
    tmp_host.file.write.assert_called_once_with(contents)

  def testWriteFileWithPush_rejected(self):
    tmp_host = MockTempFile('/tmp/file/on.host')
    contents = 'some interesting contents'
    with self.assertCalls(
        (mock.call.tempfile.NamedTemporaryFile(mode='w+'), tmp_host),
        (self.call.adb.Push('/tmp/file/on.host',
                            '/path/to/device/file'), self.CommandError())):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device._WriteFileWithPush('/path/to/device/file', contents)

  def testWriteFile_withPush(self):
    contents = 'some large contents ' * 26  # 20 * 26 = 520 chars
    with self.assertCalls(
        self.call.device._WriteFileWithPush('/path/to/device/file', contents)):
      self.device.WriteFile('/path/to/device/file', contents)

  def testWriteFile_withPushForced(self):
    contents = 'tiny contents'
    with self.assertCalls(
        self.call.device._WriteFileWithPush('/path/to/device/file', contents)):
      self.device.WriteFile('/path/to/device/file', contents, force_push=True)

  def testWriteFile_withPushAndSU(self):
    contents = 'some large contents ' * 26  # 20 * 26 = 520 chars
    with self.assertCalls(
        (self.call.device.NeedsSU(), True),
        (mock.call.devil.android.device_temp_file.DeviceTempFile(self.adb),
         MockTempFile('/sdcard/tmp/on.device')),
        self.call.device._WriteFileWithPush('/sdcard/tmp/on.device', contents),
        self.call.device.RunShellCommand(
            ['cp', '/sdcard/tmp/on.device', '/path/to/device/file'],
            as_root=True,
            check_return=True)):
      self.device.WriteFile('/path/to/device/file', contents, as_root=True)

  def testWriteFile_withEcho(self):
    with self.assertCall(
        self.call.adb.Shell("echo -n the.contents > /test/file/to.write"), ''):
      self.device.WriteFile('/test/file/to.write', 'the.contents')

  def testWriteFile_withEchoAndQuotes(self):
    with self.assertCall(
        self.call.adb.Shell("echo -n 'the contents' > '/test/file/to write'"),
        ''):
      self.device.WriteFile('/test/file/to write', 'the contents')

  def testWriteFile_withEchoAndSU(self):
    expected_cmd_without_su = "sh -c 'echo -n contents > /test/file'"
    expected_cmd = 'su -c %s' % expected_cmd_without_su
    with self.assertCalls(
        (self.call.device.NeedsSU(), True),
        (self.call.device._Su(expected_cmd_without_su), expected_cmd),
        (self.call.adb.Shell(expected_cmd), '')):
      self.device.WriteFile('/test/file', 'contents', as_root=True)


class DeviceUtilsStatDirectoryTest(DeviceUtilsTest):
  # Note: Also tests ListDirectory in testStatDirectory_fileList.

  EXAMPLE_LS_OUTPUT = [
      'total 12345',
      'drwxr-xr-x  19 root   root          0 1970-04-06 18:03 .',
      'drwxr-xr-x  19 root   root          0 1970-04-06 18:03 ..',
      'drwxr-xr-x   6 root   root            1970-01-01 00:00 some_dir',
      '-rw-r--r--   1 root   root        723 1971-01-01 07:04 some_file',
      '-rw-r-----   1 root   root        327 2009-02-13 23:30 My Music File',
      # Some Android versions escape spaces in file names
      '-rw-rw-rw-   1 root   root          0 2018-01-11 13:35 Local\\ State',
      # Older Android versions do not print st_nlink
      'lrwxrwxrwx root     root              1970-01-01 00:00 lnk -> /a/path',
      'srwxrwx--- system   system            2016-05-31 17:25 a_socket1',
      'drwxrwxrwt system   misc              1970-11-23 02:25 tmp',
      'drwxr-s--- system   shell             1970-11-23 02:24 my_cmd',
      'cr--r----- root     system    10, 183 1971-01-01 07:04 random',
      'brw------- root     root       7,   0 1971-01-01 07:04 block_dev',
      '-rwS------ root     shell      157404 2015-04-13 15:44 silly',
  ]

  FILENAMES = [
      'some_dir', 'some_file', 'My Music File', 'Local State', 'lnk',
      'a_socket1', 'tmp', 'my_cmd', 'random', 'block_dev', 'silly'
  ]

  def getStatEntries(self, path_given='/', path_listed='/'):
    with self.assertCall(
        self.call.device.RunShellCommand(['ls', '-a', '-l', path_listed],
                                         check_return=True,
                                         as_root=False,
                                         env={'TZ': 'utc'}),
        self.EXAMPLE_LS_OUTPUT):
      entries = self.device.StatDirectory(path_given)
    return {f['filename']: f for f in entries}

  def getListEntries(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['ls', '-a', '-l', '/'],
                                         check_return=True,
                                         as_root=False,
                                         env={'TZ': 'utc'}),
        self.EXAMPLE_LS_OUTPUT):
      return self.device.ListDirectory('/')

  def testStatDirectory_forceTrailingSlash(self):
    self.getStatEntries(path_given='/foo/bar/', path_listed='/foo/bar/')
    self.getStatEntries(path_given='/foo/bar', path_listed='/foo/bar/')

  def testStatDirectory_fileList(self):
    self.safeAssertItemsEqual(self.getStatEntries().keys(), self.FILENAMES)
    self.safeAssertItemsEqual(self.getListEntries(), self.FILENAMES)

  def testStatDirectory_fileModes(self):
    expected_modes = (
        ('some_dir', stat.S_ISDIR),
        ('some_file', stat.S_ISREG),
        ('lnk', stat.S_ISLNK),
        ('a_socket1', stat.S_ISSOCK),
        ('block_dev', stat.S_ISBLK),
        ('random', stat.S_ISCHR),
    )
    entries = self.getStatEntries()
    for filename, check in expected_modes:
      self.assertTrue(check(entries[filename]['st_mode']))

  def testStatDirectory_filePermissions(self):
    should_have = (
        ('some_file', stat.S_IWUSR),  # Owner can write.
        ('tmp', stat.S_IXOTH),  # Others can execute.
        ('tmp', stat.S_ISVTX),  # Has sticky bit.
        ('my_cmd', stat.S_ISGID),  # Has set-group-ID bit.
        ('silly', stat.S_ISUID),  # Has set UID bit.
    )
    should_not_have = (
        ('some_file', stat.S_IWOTH),  # Others can't write.
        ('block_dev', stat.S_IRGRP),  # Group can't read.
        ('silly', stat.S_IXUSR),  # Owner can't execute.
    )
    entries = self.getStatEntries()
    for filename, bit in should_have:
      self.assertTrue(entries[filename]['st_mode'] & bit)
    for filename, bit in should_not_have:
      self.assertFalse(entries[filename]['st_mode'] & bit)

  def testStatDirectory_numHardLinks(self):
    entries = self.getStatEntries()
    self.assertEqual(entries['some_dir']['st_nlink'], 6)
    self.assertEqual(entries['some_file']['st_nlink'], 1)
    self.assertFalse('st_nlink' in entries['tmp'])

  def testStatDirectory_fileOwners(self):
    entries = self.getStatEntries()
    self.assertEqual(entries['some_dir']['st_owner'], 'root')
    self.assertEqual(entries['my_cmd']['st_owner'], 'system')
    self.assertEqual(entries['my_cmd']['st_group'], 'shell')
    self.assertEqual(entries['tmp']['st_group'], 'misc')

  def testStatDirectory_fileSize(self):
    entries = self.getStatEntries()
    self.assertEqual(entries['some_file']['st_size'], 723)
    self.assertEqual(entries['My Music File']['st_size'], 327)
    # Sizes are sometimes not reported for non-regular files, don't try to
    # guess the size in those cases.
    self.assertFalse('st_size' in entries['some_dir'])

  def testStatDirectory_fileDateTime(self):
    entries = self.getStatEntries()
    self.assertEqual(entries['some_dir']['st_mtime'], 0)  # Epoch!
    self.assertEqual(entries['My Music File']['st_mtime'], 1234567800)

  def testStatDirectory_deviceType(self):
    entries = self.getStatEntries()
    self.assertEqual(entries['random']['st_rdev_pair'], (10, 183))
    self.assertEqual(entries['block_dev']['st_rdev_pair'], (7, 0))

  def testStatDirectory_symbolicLinks(self):
    entries = self.getStatEntries()
    self.assertEqual(entries['lnk']['symbolic_link_to'], '/a/path')
    for d in entries.values():
      self.assertEqual('symbolic_link_to' in d, stat.S_ISLNK(d['st_mode']))


class DeviceUtilsStatPathTest(DeviceUtilsTest):

  EXAMPLE_DIRECTORY = [{
      'filename': 'foo.txt',
      'st_size': 123,
      'st_time': 456
  }, {
      'filename': 'some_dir',
      'st_time': 0
  }]
  INDEX = {e['filename']: e for e in EXAMPLE_DIRECTORY}

  def testStatPath_file(self):
    with self.assertCall(
        self.call.device.StatDirectory('/data/local/tmp', as_root=False),
        self.EXAMPLE_DIRECTORY):
      self.assertEqual(self.INDEX['foo.txt'],
                       self.device.StatPath('/data/local/tmp/foo.txt'))

  def testStatPath_directory(self):
    with self.assertCall(
        self.call.device.StatDirectory('/data/local/tmp', as_root=False),
        self.EXAMPLE_DIRECTORY):
      self.assertEqual(self.INDEX['some_dir'],
                       self.device.StatPath('/data/local/tmp/some_dir'))

  def testStatPath_directoryWithTrailingSlash(self):
    with self.assertCall(
        self.call.device.StatDirectory('/data/local/tmp', as_root=False),
        self.EXAMPLE_DIRECTORY):
      self.assertEqual(self.INDEX['some_dir'],
                       self.device.StatPath('/data/local/tmp/some_dir/'))

  def testStatPath_doesNotExist(self):
    with self.assertCall(
        self.call.device.StatDirectory('/data/local/tmp', as_root=False),
        self.EXAMPLE_DIRECTORY):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.StatPath('/data/local/tmp/does.not.exist.txt')


class DeviceUtilsFileSizeTest(DeviceUtilsTest):

  EXAMPLE_DIRECTORY = [{
      'filename': 'foo.txt',
      'st_size': 123,
      'st_mtime': 456
  }, {
      'filename': 'some_dir',
      'st_mtime': 0
  }]

  def testFileSize_file(self):
    with self.assertCall(
        self.call.device.StatDirectory('/data/local/tmp', as_root=False),
        self.EXAMPLE_DIRECTORY):
      self.assertEqual(123, self.device.FileSize('/data/local/tmp/foo.txt'))

  def testFileSize_doesNotExist(self):
    with self.assertCall(
        self.call.device.StatDirectory('/data/local/tmp', as_root=False),
        self.EXAMPLE_DIRECTORY):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.FileSize('/data/local/tmp/does.not.exist.txt')

  def testFileSize_directoryWithNoSize(self):
    with self.assertCall(
        self.call.device.StatDirectory('/data/local/tmp', as_root=False),
        self.EXAMPLE_DIRECTORY):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.FileSize('/data/local/tmp/some_dir')


class DeviceUtilsSetJavaAssertsTest(DeviceUtilsTest):
  def testSetJavaAsserts_enable(self):
    with self.assertCalls(
        (self.call.device.ReadFile(self.device.LOCAL_PROPERTIES_PATH),
         'some.example.prop=with an example value\n'
         'some.other.prop=value_ok\n'),
        self.call.device.WriteFile(
            self.device.LOCAL_PROPERTIES_PATH,
            'some.example.prop=with an example value\n'
            'some.other.prop=value_ok\n'
            'dalvik.vm.enableassertions=all\n'),
        (self.call.device.GetProp('dalvik.vm.enableassertions'), ''),
        self.call.device.SetProp('dalvik.vm.enableassertions', 'all')):
      self.assertTrue(self.device.SetJavaAsserts(True))

  def testSetJavaAsserts_disable(self):
    with self.assertCalls(
        (self.call.device.ReadFile(self.device.LOCAL_PROPERTIES_PATH),
         'some.example.prop=with an example value\n'
         'dalvik.vm.enableassertions=all\n'
         'some.other.prop=value_ok\n'),
        self.call.device.WriteFile(
            self.device.LOCAL_PROPERTIES_PATH,
            'some.example.prop=with an example value\n'
            'some.other.prop=value_ok\n'),
        (self.call.device.GetProp('dalvik.vm.enableassertions'), 'all'),
        self.call.device.SetProp('dalvik.vm.enableassertions', '')):
      self.assertTrue(self.device.SetJavaAsserts(False))

  def testSetJavaAsserts_alreadyEnabled(self):
    with self.assertCalls(
        (self.call.device.ReadFile(self.device.LOCAL_PROPERTIES_PATH),
         'some.example.prop=with an example value\n'
         'dalvik.vm.enableassertions=all\n'
         'some.other.prop=value_ok\n'),
        (self.call.device.GetProp('dalvik.vm.enableassertions'), 'all')):
      self.assertFalse(self.device.SetJavaAsserts(True))

  def testSetJavaAsserts_malformedLocalProp(self):
    with self.assertCalls(
        (self.call.device.ReadFile(self.device.LOCAL_PROPERTIES_PATH),
         'some.example.prop=with an example value\n'
         'malformed_property\n'
         'dalvik.vm.enableassertions=all\n'
         'some.other.prop=value_ok\n'),
        (self.call.device.GetProp('dalvik.vm.enableassertions'), 'all')):
      self.assertFalse(self.device.SetJavaAsserts(True))


class DeviceUtilsEnsureCacheInitializedTest(DeviceUtilsTest):
  def testEnsureCacheInitialized_noCache_success(self):
    self.assertIsNone(self.device._cache['token'])
    with self.assertCall(
        self.call.device.RunShellCommand(
            AnyStringWith('getprop'),
            shell=True,
            check_return=True,
            large_output=True), ['/sdcard', 'TOKEN']):
      self.device._EnsureCacheInitialized()
    self.assertIsNotNone(self.device._cache['token'])

  def testEnsureCacheInitialized_noCache_failure(self):
    self.assertIsNone(self.device._cache['token'])
    with self.assertCall(
        self.call.device.RunShellCommand(
            AnyStringWith('getprop'),
            shell=True,
            check_return=True,
            large_output=True), self.TimeoutError()):
      with self.assertRaises(device_errors.CommandTimeoutError):
        self.device._EnsureCacheInitialized()
    self.assertIsNone(self.device._cache['token'])

  def testEnsureCacheInitialized_cache(self):
    self.device._cache['token'] = 'TOKEN'
    with self.assertCalls():
      self.device._EnsureCacheInitialized()
    self.assertIsNotNone(self.device._cache['token'])


class DeviceUtilsGetPropTest(DeviceUtilsTest):
  def testGetProp_exists(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['getprop', 'test.property'],
                                         check_return=True,
                                         single_line=True,
                                         timeout=self.device._default_timeout,
                                         retries=self.device._default_retries),
        'property_value'):
      self.assertEqual('property_value', self.device.GetProp('test.property'))

  def testGetProp_doesNotExist(self):
    with self.assertCall(
        self.call.device.RunShellCommand(['getprop', 'property.does.not.exist'],
                                         check_return=True,
                                         single_line=True,
                                         timeout=self.device._default_timeout,
                                         retries=self.device._default_retries),
        ''):
      self.assertEqual('', self.device.GetProp('property.does.not.exist'))

  def testGetProp_cachedRoProp(self):
    with self.assertCalls(
        self.EnsureCacheInitialized(props=['[ro.build.type]: [userdebug]'])):
      self.assertEqual('userdebug',
                       self.device.GetProp('ro.build.type', cache=True))
      self.assertEqual('userdebug',
                       self.device.GetProp('ro.build.type', cache=True))


class DeviceUtilsSetPropTest(DeviceUtilsTest):
  def testSetProp(self):
    with self.assertCall(
        self.call.device.RunShellCommand(
            ['setprop', 'test.property', 'test value'], check_return=True)):
      self.device.SetProp('test.property', 'test value')

  def testSetProp_check_succeeds(self):
    with self.assertCalls(
        (self.call.device.RunShellCommand(
            ['setprop', 'test.property', 'new_value'], check_return=True)),
        (self.call.device.GetProp('test.property', cache=False), 'new_value')):
      self.device.SetProp('test.property', 'new_value', check=True)

  def testSetProp_check_fails(self):
    with self.assertCalls(
        (self.call.device.RunShellCommand(
            ['setprop', 'test.property', 'new_value'], check_return=True)),
        (self.call.device.GetProp('test.property', cache=False), 'old_value')):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.SetProp('test.property', 'new_value', check=True)


class DeviceUtilsListProcessesTest(DeviceUtilsTest):
  def setUp(self):
    super(DeviceUtilsListProcessesTest, self).setUp()
    self.sample_output = [
        'USER  PID     PPID  VSIZE RSS   WCHAN          PC  NAME',
        'user  1001    100   1024  1024  ffffffff 00000000 one.match',
        'user  1002    100   1024  1024  ffffffff 00000000 two.match',
        'user  1003    101   1024  1024  ffffffff 00000000 three.match',
        'user  1234    101   1024  1024  ffffffff 00000000 my$process',
        'user  1236    100   1024  1024  ffffffff 00000000 foo',
        'user  1578    1236  1024  1024  ffffffff 00000000 foo',
    ]

  def _grepOutput(self, substring):
    return [line for line in self.sample_output if substring in line]

  def testListProcesses_sdkGreaterThanNougatMR1(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=(version_codes.NOUGAT_MR1 + 1)):
      with self.patch_call(self.call.device.build_id, return_value='ZZZ99Z'):
        with self.assertCall(
            self.call.device._RunPipedShellCommand(
                'ps -e | grep -F example.process'), []):
          self.device.ListProcesses('example.process')

  def testListProcesses_noMatches(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCall(
          self.call.device._RunPipedShellCommand('ps | grep -F does.not.match'),
          self._grepOutput('does.not.match')):
        self.assertEqual([], self.device.ListProcesses('does.not.match'))

  def testListProcesses_oneMatch(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCall(
          self.call.device._RunPipedShellCommand('ps | grep -F one.match'),
          self._grepOutput('one.match')):
        self.assertEqual(
            Processes(('one.match', 1001, 100)),
            self.device.ListProcesses('one.match'))

  def testListProcesses_multipleMatches(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCall(
          self.call.device._RunPipedShellCommand('ps | grep -F match'),
          self._grepOutput('match')):
        self.assertEqual(
            Processes(('one.match', 1001, 100), ('two.match', 1002, 100),
                      ('three.match', 1003, 101)),
            self.device.ListProcesses('match'))

  def testListProcesses_quotable(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCall(
          self.call.device._RunPipedShellCommand("ps | grep -F 'my$process'"),
          self._grepOutput('my$process')):
        self.assertEqual(
            Processes(('my$process', 1234, 101)),
            self.device.ListProcesses('my$process'))

  # Tests for the GetPids wrapper interface.
  def testGetPids_multipleInstances(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCall(
          self.call.device._RunPipedShellCommand('ps | grep -F foo'),
          self._grepOutput('foo')):
        self.assertEqual({'foo': ['1236', '1578']}, self.device.GetPids('foo'))

  def testGetPids_allProcesses(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCall(
          self.call.device.RunShellCommand(['ps'],
                                           check_return=True,
                                           large_output=True),
          self.sample_output):
        self.assertEqual({
            'one.match': ['1001'],
            'two.match': ['1002'],
            'three.match': ['1003'],
            'my$process': ['1234'],
            'foo': ['1236', '1578']
        }, self.device.GetPids())

  # Tests for the GetApplicationPids wrapper interface.
  def testGetApplicationPids_notFound(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCall(
          self.call.device._RunPipedShellCommand('ps | grep -F match'),
          self._grepOutput('match')):
        # No PIDs found, process name should be exact match.
        self.assertEqual([], self.device.GetApplicationPids('match'))

  def testGetApplicationPids_foundOne(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCall(
          self.call.device._RunPipedShellCommand('ps | grep -F one.match'),
          self._grepOutput('one.match')):
        self.assertEqual([1001], self.device.GetApplicationPids('one.match'))

  def testGetApplicationPids_foundMany(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCall(
          self.call.device._RunPipedShellCommand('ps | grep -F foo'),
          self._grepOutput('foo')):
        self.assertEqual([1236, 1578], self.device.GetApplicationPids('foo'))

  def testGetApplicationPids_atMostOneNotFound(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCall(
          self.call.device._RunPipedShellCommand('ps | grep -F match'),
          self._grepOutput('match')):
        # No PIDs found, process name should be exact match.
        self.assertEqual(
            None, self.device.GetApplicationPids('match', at_most_one=True))

  def testGetApplicationPids_atMostOneFound(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCall(
          self.call.device._RunPipedShellCommand('ps | grep -F one.match'),
          self._grepOutput('one.match')):
        self.assertEqual(
            1001, self.device.GetApplicationPids('one.match', at_most_one=True))

  def testGetApplicationPids_atMostOneFoundTooMany(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertRaises(device_errors.CommandFailedError):
        with self.assertCall(
            self.call.device._RunPipedShellCommand('ps | grep -F foo'),
            self._grepOutput('foo')):
          self.device.GetApplicationPids('foo', at_most_one=True)


class DeviceUtilsGetSetEnforce(DeviceUtilsTest):
  def testGetEnforce_Enforcing(self):
    with self.assertCall(self.call.adb.Shell('getenforce'), 'Enforcing'):
      self.assertEqual(True, self.device.GetEnforce())

  def testGetEnforce_Permissive(self):
    with self.assertCall(self.call.adb.Shell('getenforce'), 'Permissive'):
      self.assertEqual(False, self.device.GetEnforce())

  def testGetEnforce_Disabled(self):
    with self.assertCall(self.call.adb.Shell('getenforce'), 'Disabled'):
      self.assertEqual(None, self.device.GetEnforce())

  def testSetEnforce_Enforcing(self):
    with self.assertCalls((self.call.device.NeedsSU(), False),
                          (self.call.adb.Shell('setenforce 1'), '')):
      self.device.SetEnforce(enabled=True)

  def testSetEnforce_Permissive(self):
    with self.assertCalls((self.call.device.NeedsSU(), False),
                          (self.call.adb.Shell('setenforce 0'), '')):
      self.device.SetEnforce(enabled=False)

  def testSetEnforce_EnforcingWithInt(self):
    with self.assertCalls((self.call.device.NeedsSU(), False),
                          (self.call.adb.Shell('setenforce 1'), '')):
      self.device.SetEnforce(enabled=1)

  def testSetEnforce_PermissiveWithInt(self):
    with self.assertCalls((self.call.device.NeedsSU(), False),
                          (self.call.adb.Shell('setenforce 0'), '')):
      self.device.SetEnforce(enabled=0)

  def testSetEnforce_EnforcingWithStr(self):
    with self.assertCalls((self.call.device.NeedsSU(), False),
                          (self.call.adb.Shell('setenforce 1'), '')):
      self.device.SetEnforce(enabled='1')

  def testSetEnforce_PermissiveWithStr(self):
    with self.assertCalls((self.call.device.NeedsSU(), False),
                          (self.call.adb.Shell('setenforce 0'), '')):
      self.device.SetEnforce(enabled='0')  # Not recommended but it works!


class DeviceUtilsGetWebViewUpdateServiceDumpTest(DeviceUtilsTest):
  def testGetWebViewUpdateServiceDump_success(self):
    # Some of the lines of adb shell dumpsys webviewupdate:
    dumpsys_lines = [
        'Fallback logic enabled: true',
        ('Current WebView package (name, version): '
         '(com.android.chrome, 61.0.3163.98)'),
        'Minimum WebView version code: 12345',
        'WebView packages:',
        ('Valid package com.android.chrome (versionName: '
         '61.0.3163.98, versionCode: 1, targetSdkVersion: 26) is  '
         'installed/enabled for all users'),
        ('Valid package com.google.android.webview (versionName: '
         '58.0.3029.125, versionCode: 1, targetSdkVersion: 26) is NOT '
         'installed/enabled for all users'),
        ('Invalid package com.google.android.apps.chrome (versionName: '
         '56.0.2924.122, versionCode: 2, targetSdkVersion: 25), reason: SDK '
         'version too low'),
        ('com.chrome.canary is NOT installed.'),
    ]
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.OREO):
      with self.assertCall(
          self.call.adb.Shell('dumpsys webviewupdate'),
          '\n'.join(dumpsys_lines)):
        update = self.device.GetWebViewUpdateServiceDump()
        self.assertTrue(update['FallbackLogicEnabled'])
        self.assertEqual('com.android.chrome', update['CurrentWebViewPackage'])
        self.assertEqual('61.0.3163.98', update['CurrentWebViewVersion'])
        self.assertEqual(12345, update['MinimumWebViewVersionCode'])
        # Order isn't really important, and we shouldn't have duplicates, so we
        # convert to sets.
        expected = {
            'com.android.chrome', 'com.google.android.webview',
            'com.google.android.apps.chrome', 'com.chrome.canary'
        }
        self.assertSetEqual(expected, set(update['WebViewPackages'].keys()))
        self.assertEqual('is  installed/enabled for all users',
                         update['WebViewPackages']['com.android.chrome'])
        self.assertEqual(
            'is NOT installed/enabled for all users',
            update['WebViewPackages']['com.google.android.webview'])
        self.assertEqual(
            'reason: SDK version too low',
            update['WebViewPackages']['com.google.android.apps.chrome'])
        self.assertEqual('is NOT installed.',
                         update['WebViewPackages']['com.chrome.canary'])

  def testGetWebViewUpdateServiceDump_missingkey(self):
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.OREO):
      with self.assertCall(
          self.call.adb.Shell('dumpsys webviewupdate'),
          'Fallback logic enabled: true'):
        update = self.device.GetWebViewUpdateServiceDump()
        self.assertEqual(True, update['FallbackLogicEnabled'])

  def testGetWebViewUpdateServiceDump_noop(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.NOUGAT_MR1):
      with self.assertCalls():
        self.device.GetWebViewUpdateServiceDump()

  def testGetWebViewUpdateServiceDump_noPackage(self):
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.OREO):
      with self.assertCall(
          self.call.adb.Shell('dumpsys webviewupdate'),
          'Fallback logic enabled: true\n'
          'Current WebView package is null'):
        update = self.device.GetWebViewUpdateServiceDump()
        self.assertEqual(True, update['FallbackLogicEnabled'])
        self.assertEqual(None, update['CurrentWebViewPackage'])


class DeviceUtilsSetWebViewImplementationTest(DeviceUtilsTest):
  def testSetWebViewImplementation_success(self):
    with self.patch_call(
        self.call.device.IsApplicationInstalled, return_value=True):
      with self.assertCall(
          self.call.adb.Shell(
              'cmd webviewupdate set-webview-implementation foo.org'),
          'Success'):
        self.device.SetWebViewImplementation('foo.org')

  def testSetWebViewImplementation_uninstalled(self):
    with self.patch_call(
        self.call.device.IsApplicationInstalled, return_value=False):
      with self.assertRaises(device_errors.CommandFailedError) as cfe:
        self.device.SetWebViewImplementation('foo.org')
      self.assertIn('is not installed', cfe.exception.message)

  def _testSetWebViewImplementationHelper(self, mock_dump_sys,
                                          exception_message_substr):
    with self.patch_call(
        self.call.device.IsApplicationInstalled, return_value=True):
      with self.assertCall(
          self.call.adb.Shell(
              'cmd webviewupdate set-webview-implementation foo.org'), 'Oops!'):
        with self.patch_call(
            self.call.device.GetWebViewUpdateServiceDump,
            return_value=mock_dump_sys):
          with self.assertRaises(device_errors.CommandFailedError) as cfe:
            self.device.SetWebViewImplementation('foo.org')
          self.assertIn(exception_message_substr, cfe.exception.message)

  def testSetWebViewImplementation_notInProviderList(self):
    mock_dump_sys = {
        'WebViewPackages': {
            'some.package': 'any reason',
            'other.package': 'any reason',
        }
    }
    self._testSetWebViewImplementationHelper(mock_dump_sys, 'provider list')

  def testSetWebViewImplementation_notEnabled(self):
    mock_dump_sys = {
        'WebViewPackages': {
            'foo.org': 'is NOT installed/enabled for all users',
        }
    }
    self._testSetWebViewImplementationHelper(mock_dump_sys, 'is disabled')

  def testSetWebViewImplementation_missingManifestTag(self):
    mock_dump_sys = {
        'WebViewPackages': {
            'foo.org': 'No WebView-library manifest flag',
        }
    }
    self._testSetWebViewImplementationHelper(mock_dump_sys,
                                             'WebView native library')

  def testSetWebViewImplementation_lowTargetSdkVersion_finalizedSdk(self):
    mock_dump_sys = {'WebViewPackages': {'foo.org': 'SDK version too low', }}
    with self.assertCalls(
        (self.call.device.GetApplicationTargetSdk('foo.org'), '29'),
        (self.call.device.GetProp('ro.build.version.preview_sdk'), '0')):
      with self.patch_call(self.call.device.build_version_sdk, return_value=30):
        self._testSetWebViewImplementationHelper(
            mock_dump_sys,
            "has targetSdkVersion '29', but valid WebView providers must "
            "target >= 30 on this device")

  def testSetWebViewImplementation_lowTargetSdkVersion_prefinalizedSdk(self):
    mock_dump_sys = {'WebViewPackages': {'foo.org': 'SDK version too low', }}
    with self.assertCalls(
        (self.call.device.GetApplicationTargetSdk('foo.org'), '29'),
        (self.call.device.GetProp('ro.build.version.preview_sdk'), '1'),
        (self.call.device.GetProp('ro.build.version.codename'), 'R')):
      with self.patch_call(self.call.device.build_version_sdk, return_value=29):
        self._testSetWebViewImplementationHelper(
            mock_dump_sys,
            "targets a finalized SDK ('29'), but valid WebView providers must "
            "target a pre-finalized SDK ('R') on this device")

  def testSetWebViewImplementation_lowVersionCode(self):
    mock_dump_sys = {
        'MinimumWebViewVersionCode': 12345,
        'WebViewPackages': {
            'foo.org': 'Version code too low',
        }
    }
    self._testSetWebViewImplementationHelper(mock_dump_sys,
                                             'higher versionCode')

  def testSetWebViewImplementation_invalidSignature(self):
    mock_dump_sys = {'WebViewPackages': {'foo.org': 'Incorrect signature'}}
    self._testSetWebViewImplementationHelper(mock_dump_sys,
                                             'signed with release keys')


class DeviceUtilsSetWebViewFallbackLogicTest(DeviceUtilsTest):
  def testSetWebViewFallbackLogic_False_success(self):
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.NOUGAT):
      with self.assertCall(
          self.call.adb.Shell('cmd webviewupdate enable-redundant-packages'),
          'Success'):
        self.device.SetWebViewFallbackLogic(False)

  def testSetWebViewFallbackLogic_True_success(self):
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.NOUGAT):
      with self.assertCall(
          self.call.adb.Shell('cmd webviewupdate disable-redundant-packages'),
          'Success'):
        self.device.SetWebViewFallbackLogic(True)

  def testSetWebViewFallbackLogic_failure(self):
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.NOUGAT):
      with self.assertCall(
          self.call.adb.Shell('cmd webviewupdate enable-redundant-packages'),
          'Oops!'):
        with self.assertRaises(device_errors.CommandFailedError):
          self.device.SetWebViewFallbackLogic(False)

  def testSetWebViewFallbackLogic_beforeNougat(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.MARSHMALLOW):
      with self.assertCalls():
        self.device.SetWebViewFallbackLogic(False)

  def testSetWebViewFallbackLogic_afterPie(self):
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.Q):
      with self.assertCalls():
        self.device.SetWebViewFallbackLogic(False)


class DeviceUtilsTakeScreenshotTest(DeviceUtilsTest):
  def testTakeScreenshot_fileNameProvided(self):
    with self.patch_call(self.call.device.product_name,
                         return_value='hammerhead'):
      with self.assertCalls(
          (mock.call.devil.android.device_temp_file.DeviceTempFile(
              self.adb, suffix='.png'), MockTempFile('/tmp/path/temp-123.png')),
          (self.call.adb.Shell(
              '/system/bin/screencap -p /tmp/path/temp-123.png'), ''),
          self.call.device.PullFile('/tmp/path/temp-123.png',
                                    '/test/host/screenshot.png')):
        self.device.TakeScreenshot('/test/host/screenshot.png')

  def testTakeScreenshot_alternateCmdDevice(self):
    with self.patch_call(self.call.device.product_name, return_value='flame'):
      with self.assertCalls(
          (mock.call.devil.android.device_temp_file.DeviceTempFile(
              self.adb, suffix='.png'), MockTempFile('/tmp/path/temp-123.png')),
          (self.call.adb.Shell(
              '/system/bin/screencap -p > /tmp/path/temp-123.png'), ''),
          self.call.device.PullFile('/tmp/path/temp-123.png',
                                    '/test/host/screenshot.png')):
        self.device.TakeScreenshot('/test/host/screenshot.png')


class DeviceUtilsDismissCrashDialogIfNeededTest(DeviceUtilsTest):
  def testDismissCrashDialogIfNeeded_crashedPackageNotFound(self):
    sample_dumpsys_output = '''
WINDOW MANAGER WINDOWS (dumpsys window windows)
  Window #11 Window{f8b647a u0 SearchPanel}:
    mDisplayId=0 mSession=Session{8 94:122} mClient=android.os.BinderProxy@1ba5
    mOwnerUid=100 mShowToOwnerOnly=false package=com.android.systemui appop=NONE
    mAttrs=WM.LayoutParams{(0,0)(fillxfill) gr=#53 sim=#31 ty=2024 fl=100
    Requested w=1080 h=1920 mLayoutSeq=426
    mBaseLayer=211000 mSubLayer=0 mAnimLayer=211000+0=211000 mLastLayer=211000
'''
    with self.assertCalls(
        (self.call.device.RunShellCommand(['dumpsys', 'window', 'windows'],
                                          check_return=True,
                                          large_output=True),
         sample_dumpsys_output.split('\n'))):
      package_name = self.device.DismissCrashDialogIfNeeded()
      self.assertIsNone(package_name)

  def testDismissCrashDialogIfNeeded_crashedPackageFound_sdk_preN(self):
    sample_dumpsys_output = '''
WINDOW MANAGER WINDOWS (dumpsys window windows)
  Window #11 Window{f8b647a u0 SearchPanel}:
    mDisplayId=0 mSession=Session{8 94:122} mClient=android.os.BinderProxy@1ba5
    mOwnerUid=102 mShowToOwnerOnly=false package=com.android.systemui appop=NONE
    mAttrs=WM.LayoutParams{(0,0)(fillxfill) gr=#53 sim=#31 ty=2024 fl=100
    Requested w=1080 h=1920 mLayoutSeq=426
    mBaseLayer=211000 mSubLayer=0 mAnimLayer=211000+0=211000 mLastLayer=211000
  mHasPermanentDpad=false
  mCurrentFocus=Window{3a27740f u0 Application Error: com.android.chrome}
  mFocusedApp=AppWindowToken{470af6f token=Token{272ec24e ActivityRecord{t894}}}
'''
    with self.assertCalls(
        (self.call.device.RunShellCommand(
            ['dumpsys', 'window', 'windows'],
            check_return=True,
            large_output=True), sample_dumpsys_output.split('\n')),
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '23'),
        (self.call.device.RunShellCommand(['input', 'keyevent', '22'],
                                          check_return=True)),
        (self.call.device.RunShellCommand(['input', 'keyevent', '22'],
                                          check_return=True)),
        (self.call.device.RunShellCommand(['input', 'keyevent', '66'],
                                          check_return=True)),
        (self.call.device.RunShellCommand(['dumpsys', 'window', 'windows'],
                                          check_return=True,
                                          large_output=True), [])):
      package_name = self.device.DismissCrashDialogIfNeeded()
      self.assertEqual(package_name, 'com.android.chrome')

  def testDismissCrashDialogIfNeeded_crashedPackageFound_sdk_N(self):
    sample_dumpsys_output = '''
WINDOW MANAGER WINDOWS (dumpsys window windows)
  Window #11 Window{f8b647a u0 SearchPanel}:
    mDisplayId=0 mSession=Session{8 94:122} mClient=android.os.BinderProxy@1ba5
    mOwnerUid=102 mShowToOwnerOnly=false package=com.android.systemui appop=NONE
    mAttrs=WM.LayoutParams{(0,0)(fillxfill) gr=#53 sim=#31 ty=2024 fl=100
    Requested w=1080 h=1920 mLayoutSeq=426
    mBaseLayer=211000 mSubLayer=0 mAnimLayer=211000+0=211000 mLastLayer=211000
  mHasPermanentDpad=false
  mCurrentFocus=Window{3a27740f u0 Application Error: com.android.chrome}
  mFocusedApp=AppWindowToken{470af6f token=Token{272ec24e ActivityRecord{t894}}}
'''
    with self.assertCalls(
        (self.call.device.RunShellCommand(
            ['dumpsys', 'window', 'windows'],
            check_return=True,
            large_output=True), sample_dumpsys_output.split('\n')),
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '25'),
        (self.call.device.RunShellCommand([
            'am', 'broadcast', '-a',
            'android.intent.action.CLOSE_SYSTEM_DIALOGS'
        ],
                                          check_return=True)),
        (self.call.device.RunShellCommand(['dumpsys', 'window', 'windows'],
                                          check_return=True,
                                          large_output=True), [])):
      package_name = self.device.DismissCrashDialogIfNeeded()
      self.assertEqual(package_name, 'com.android.chrome')


class DeviceUtilsClientCache(DeviceUtilsTest):
  def testClientCache_twoCaches(self):
    self.device._cache['test'] = 0
    client_cache_one = self.device.GetClientCache('ClientOne')
    client_cache_one['test'] = 1
    client_cache_two = self.device.GetClientCache('ClientTwo')
    client_cache_two['test'] = 2
    self.assertEqual(self.device._cache['test'], 0)
    self.assertEqual(client_cache_one, {'test': 1})
    self.assertEqual(client_cache_two, {'test': 2})
    self.device.ClearCache()
    self.assertTrue('test' not in self.device._cache)
    self.assertEqual(client_cache_one, {})
    self.assertEqual(client_cache_two, {})

  def testClientCache_multipleInstances(self):
    client_cache_one = self.device.GetClientCache('ClientOne')
    client_cache_one['test'] = 1
    client_cache_two = self.device.GetClientCache('ClientOne')
    self.assertEqual(client_cache_one, {'test': 1})
    self.assertEqual(client_cache_two, {'test': 1})
    self.device.ClearCache()
    self.assertEqual(client_cache_one, {})
    self.assertEqual(client_cache_two, {})


class DeviceUtilsHealthyDevicesTest(mock_calls.TestCase):
  @mock.patch('devil.android.sdk.adb_wrapper.AdbWrapper.is_ready',
              return_value=True)
  def testHealthyDevices_emptyDenylist_defaultDeviceArg(self, _mock_get_state):
    test_serials = ['0123456789abcdef', 'fedcba9876543210']
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM])):
      denylist = mock.NonCallableMock(**{'Read.return_value': []})
      devices = device_utils.DeviceUtils.HealthyDevices(denylist)
    for serial, device in zip(test_serials, devices):
      self.assertTrue(isinstance(device, device_utils.DeviceUtils))
      self.assertEqual(serial, device.adb.GetDeviceSerial())

  @mock.patch('devil.android.sdk.adb_wrapper.AdbWrapper.is_ready',
              return_value=True)
  def testHealthyDevices_denylist_defaultDeviceArg(self, _mock_get_state):
    test_serials = ['0123456789abcdef', 'fedcba9876543210']
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM])):
      denylist = mock.NonCallableMock(
          **{'Read.return_value': ['fedcba9876543210']})
      devices = device_utils.DeviceUtils.HealthyDevices(denylist)
    self.assertEqual(1, len(devices))
    self.assertTrue(isinstance(devices[0], device_utils.DeviceUtils))
    self.assertEqual('0123456789abcdef', devices[0].adb.GetDeviceSerial())

  @mock.patch('devil.android.sdk.adb_wrapper.AdbWrapper.is_ready',
              return_value=True)
  def testHealthyDevices_noneDeviceArg_multiple_attached(self, _mock_get_state):
    test_serials = ['0123456789abcdef', 'fedcba9876543210']
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM]),
        (mock.call.devil.android.device_errors.MultipleDevicesError(
            mock.ANY), _MockMultipleDevicesError())):
      with self.assertRaises(_MockMultipleDevicesError):
        device_utils.DeviceUtils.HealthyDevices(device_arg=None)

  def testHealthyDevices_noneDeviceArg_one_attached(self):
    test_serials = ['0123456789abcdef']
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM])):
      devices = device_utils.DeviceUtils.HealthyDevices(device_arg=None)
    self.assertEqual(1, len(devices))

  def testHealthyDevices_noneDeviceArg_one_attached_old_props(self):
    test_serials = ['0123456789abcdef']
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         []),
        (mock.call.devil.android.device_utils.DeviceUtils.GetABI(), [abis.ARM
                                                                     ])):
      devices = device_utils.DeviceUtils.HealthyDevices(device_arg=None)
    self.assertEqual(1, len(devices))

  def testHealthyDevices_noneDeviceArg_one_attached_multi_abi(self):
    test_serials = ['0123456789abcdef']
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM, abis.X86])):
      devices = device_utils.DeviceUtils.HealthyDevices(device_arg=None,
                                                        abis=[abis.X86])
    self.assertEqual(1, len(devices))

  def testHealthyDevices_noneDeviceArg_no_attached(self):
    test_serials = []
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials])):
      with self.assertRaises(device_errors.NoDevicesError):
        device_utils.DeviceUtils.HealthyDevices(device_arg=None, retries=0)

  @mock.patch('devil.android.sdk.adb_wrapper.AdbWrapper.is_ready',
              return_value=True)
  def testHealthyDevices_noneDeviceArg_multiple_attached_ANDROID_SERIAL(
      self, _mock_get_state):
    try:
      os.environ['ANDROID_SERIAL'] = '0123456789abcdef'
      with self.assertCalls():  # Should skip adb devices when device is known.
        device_utils.DeviceUtils.HealthyDevices(device_arg=None)
    finally:
      del os.environ['ANDROID_SERIAL']

  @mock.patch('devil.android.sdk.adb_wrapper.AdbWrapper.is_ready',
              return_value=True)
  def testHealthyDevices_stringDeviceArg(self, _mock_get_state):
    with self.assertCalls():  # Should skip adb devices when device is known.
      devices = device_utils.DeviceUtils.HealthyDevices(
          device_arg='0123456789abcdef')
    self.assertEqual(1, len(devices))

  def testHealthyDevices_EmptyListDeviceArg_multiple_attached(self):
    test_serials = ['0123456789abcdef', 'fedcba9876543210']
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM])):
      devices = device_utils.DeviceUtils.HealthyDevices(device_arg=())
    self.assertEqual(2, len(devices))

  def testHealthyDevices_EmptyListDeviceArg_multiple_attached_multi_abi(self):
    test_serials = ['0123456789abcdef', 'fedcba9876543210']
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.X86]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM, abis.X86])):
      devices = device_utils.DeviceUtils.HealthyDevices(device_arg=(),
                                                        abis=[abis.X86])
    self.assertEqual(2, len(devices))

  @mock.patch('devil.android.sdk.adb_wrapper.AdbWrapper.is_ready',
              return_value=True)
  def testHealthyDevices_EmptyListDeviceArg_ANDROID_SERIAL(
      self, _mock_get_state):
    try:
      os.environ['ANDROID_SERIAL'] = '0123456789abcdef'
      with self.assertCalls():  # Should skip adb devices when device is known.
        devices = device_utils.DeviceUtils.HealthyDevices(device_arg=())
    finally:
      del os.environ['ANDROID_SERIAL']
    self.assertEqual(1, len(devices))

  def testHealthyDevices_EmptyListDeviceArg_no_attached(self):
    test_serials = []
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials])):
      with self.assertRaises(device_errors.NoDevicesError):
        device_utils.DeviceUtils.HealthyDevices(device_arg=[], retries=0)

  @mock.patch('time.sleep')
  @mock.patch('devil.android.sdk.adb_wrapper.RestartServer')
  def testHealthyDevices_EmptyListDeviceArg_no_attached_with_retry(
      self, mock_restart, mock_sleep):
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY), []),
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY), []),
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY), []),
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY), []),
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY), [])):
      with self.assertRaises(device_errors.NoDevicesError):
        device_utils.DeviceUtils.HealthyDevices(device_arg=[], retries=4)
    self.assertEqual(mock_restart.call_count, 4)
    self.assertEqual(
        mock_sleep.call_args_list,
        [mock.call(2), mock.call(4),
         mock.call(8), mock.call(16)])

  @mock.patch('time.sleep')
  @mock.patch('devil.android.sdk.adb_wrapper.RestartServer')
  def testHealthyDevices_EmptyListDeviceArg_no_attached_with_resets(
      self, mock_restart, mock_sleep):
    # The reset_usb import fails on windows. Mock the full import here so it can
    # succeed like it would on linux.
    mock_reset_import = mock.MagicMock()
    sys.modules['devil.utils.reset_usb'] = mock_reset_import
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY), []),
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY), []),
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY), []),
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY), []),
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY), [])):
      with self.assertRaises(device_errors.NoDevicesError):
        with mock.patch.object(mock_reset_import,
                               'reset_all_android_devices') as mock_reset:
          device_utils.DeviceUtils.HealthyDevices(
              device_arg=[], retries=4, enable_usb_resets=True)
          self.assertEqual(mock_reset.call_count, 1)
    self.assertEqual(mock_restart.call_count, 4)
    self.assertEqual(
        mock_sleep.call_args_list,
        [mock.call(2), mock.call(4),
         mock.call(8), mock.call(16)])

  @mock.patch('devil.android.sdk.adb_wrapper.AdbWrapper.is_ready',
              return_value=True)
  def testHealthyDevices_ListDeviceArg(self, _mock_get_state):
    device_arg = ['0123456789abcdef', 'fedcba9876543210']
    try:
      os.environ['ANDROID_SERIAL'] = 'should-not-apply'
      with self.assertCalls():  # Should skip adb devices when device is known.
        devices = device_utils.DeviceUtils.HealthyDevices(device_arg=device_arg)
    finally:
      del os.environ['ANDROID_SERIAL']
    self.assertEqual(2, len(devices))

  def testHealthyDevices_abisArg_no_matching_abi(self):
    test_serials = ['0123456789abcdef', 'fedcba9876543210']
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM])):
      with self.assertRaises(device_errors.NoDevicesError):
        device_utils.DeviceUtils.HealthyDevices(
            device_arg=[], retries=0, abis=[abis.ARM_64])

  def testHealthyDevices_abisArg_filter_on_abi(self):
    test_serials = ['0123456789abcdef', 'fedcba9876543210']
    with self.assertCalls(
        (mock.call.devil.android.sdk.adb_wrapper.AdbWrapper.Devices(
            persistent_shell=mock.ANY),
         [_AdbWrapperMock(s) for s in test_serials]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM_64]),
        (mock.call.devil.android.device_utils.DeviceUtils.GetSupportedABIs(),
         [abis.ARM])):
      devices = device_utils.DeviceUtils.HealthyDevices(
          device_arg=[], retries=0, abis=[abis.ARM_64])
    self.assertEqual(1, len(devices))


class DeviceUtilsRestartAdbdTest(DeviceUtilsTest):
  def testAdbdRestart(self):
    with self.assertCalls((self.call.device.RunShellCommand(
        ['setprop', 'ctl.restart', 'adbd'], check_return=False, as_root=True)),
                          self.call.adb.KillAllPersistentAdbs(),
                          self.call.adb.WaitForDevice()):
      self.device.RestartAdbd()


class DeviceUtilsGrantPermissionsTest(DeviceUtilsTest):
  def _PmGrantShellCall(self, package, permissions):
    fragment = 'p=%s;for q in %s;' % (package, ' '.join(sorted(permissions)))
    results = []
    for permission, result in sorted(permissions.items()):
      if result:
        output, status = result + '\n', 1
      else:
        output, status = '', 0
      results.append('{output}{sep}{permission}{sep}{status}{sep}\n'.format(
          output=output,
          permission=permission,
          status=status,
          sep=device_utils._SHELL_OUTPUT_SEPARATOR))
    return (self.call.device.RunShellCommand(
        AnyStringWith(fragment),
        shell=True,
        raw_output=True,
        large_output=True,
        check_return=True), ''.join(results))

  def testGrantPermissions_none(self):
    self.device.GrantPermissions('package', [])

  def testGrantPermissions_one(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.MARSHMALLOW):
      with self.assertCalls(self._PmGrantShellCall('package', {'p1': 0})):
        self.device.GrantPermissions('package', ['p1'])

  def testGrantPermissions_multiple(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.MARSHMALLOW):
      with self.assertCalls(
          self._PmGrantShellCall('package', {
              'p1': 0,
              'p2': 0
          })):
        self.device.GrantPermissions('package', ['p1', 'p2'])

  def testGrantPermissions_currentUser(self):
    with self.patch_call(self.call.device.build_version_sdk,
                         return_value=version_codes.MARSHMALLOW), \
         self.patch_call(self.call.device.target_user,
                         return_value=11):
      with self.assertCalls((self.call.device.RunShellCommand(
          AnyStringWith('do pm grant --user 11'),
          shell=True,
          raw_output=True,
          large_output=True,
          check_return=True), '{sep}p1{sep}0{sep}'.format(
              sep=device_utils._SHELL_OUTPUT_SEPARATOR))):
        self.device.GrantPermissions('package', ['p1'])

  def testGrantPermissions_WriteExtrnalStorage(self):
    WRITE = 'android.permission.WRITE_EXTERNAL_STORAGE'
    READ = 'android.permission.READ_EXTERNAL_STORAGE'
    with PatchLogger() as logger:
      with self.patch_call(
          self.call.device.build_version_sdk,
          return_value=version_codes.MARSHMALLOW):
        with self.assertCalls(
            self._PmGrantShellCall('package', {
                READ: 0,
                WRITE: 0
            })):
          self.device.GrantPermissions('package', [WRITE])
      self.assertEqual(logger.warnings, [])

  def testGrantPermissions_ManageExtrnalStorage(self):
    with PatchLogger() as logger:
      with self.patch_call(self.call.device.build_version_sdk,
                           return_value=version_codes.R):
        with self.assertCalls(
            (self.call.device.RunShellCommand(
                AnyStringWith('appops set  pkg MANAGE_EXTERNAL_STORAGE allow'),
                shell=True,
                raw_output=True,
                large_output=True,
                check_return=True),
             '{sep}MANAGE_EXTERNAL_STORAGE{sep}0{sep}\n'.format(
                 sep=device_utils._SHELL_OUTPUT_SEPARATOR))):
          self.device.GrantPermissions(
              'pkg', ['android.permission.MANAGE_EXTERNAL_STORAGE'])
      self.assertEqual(logger.warnings, [])

  def testGrantPermissions_ManageExtrnalStorage_currentUser(self):
    with PatchLogger() as logger:
      with self.patch_call(self.call.device.build_version_sdk,
                           return_value=version_codes.R), \
           self.patch_call(self.call.device.target_user,
                           return_value=11):
        with self.assertCalls(
            (self.call.device.RunShellCommand(AnyStringWith(
                'appops set --user 11 '
                'pkg MANAGE_EXTERNAL_STORAGE allow'),
                                              shell=True,
                                              raw_output=True,
                                              large_output=True,
                                              check_return=True),
             '{sep}MANAGE_EXTERNAL_STORAGE{sep}0{sep}\n'.format(
                 sep=device_utils._SHELL_OUTPUT_SEPARATOR))):
          self.device.GrantPermissions(
              'pkg', ['android.permission.MANAGE_EXTERNAL_STORAGE'])
      self.assertEqual(logger.warnings, [])

  def testGrantPermissions_DenyList(self):
    with PatchLogger() as logger:
      with self.patch_call(
          self.call.device.build_version_sdk,
          return_value=version_codes.MARSHMALLOW):
        with self.assertCalls(self._PmGrantShellCall('package', {'p1': 0})):
          self.device.GrantPermissions('package',
                                       ['p1', 'foo.permission.C2D_MESSAGE'])
      self.assertEqual(logger.warnings, [])

  def testGrantPermissions_unchangeablePermision(self):
    error_message = (
        'Operation not allowed: java.lang.SecurityException: '
        'Permission UNCHANGEABLE is not a changeable permission type')
    with PatchLogger() as logger:
      with self.patch_call(
          self.call.device.build_version_sdk,
          return_value=version_codes.MARSHMALLOW):
        with self.assertCalls(
            self._PmGrantShellCall('package', {'UNCHANGEABLE': error_message})):
          self.device.GrantPermissions('package', ['UNCHANGEABLE'])
      self.assertEqual(logger.warnings,
                       [mock.ANY, AnyStringWith('UNCHANGEABLE')])


class DeviecUtilsIsScreenOn(DeviceUtilsTest):

  _L_SCREEN_ON = ['test=test mInteractive=true']
  _K_SCREEN_ON = ['test=test mScreenOn=true']
  _L_SCREEN_OFF = ['mInteractive=false']
  _K_SCREEN_OFF = ['mScreenOn=false']

  def testIsScreenOn_onPreL(self):
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.KITKAT):
      with self.assertCalls((self.call.device._RunPipedShellCommand(
          'dumpsys input_method | grep mScreenOn'), self._K_SCREEN_ON)):
        self.assertTrue(self.device.IsScreenOn())

  def testIsScreenOn_onL(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCalls((self.call.device._RunPipedShellCommand(
          'dumpsys input_method | grep mInteractive'), self._L_SCREEN_ON)):
        self.assertTrue(self.device.IsScreenOn())

  def testIsScreenOn_offPreL(self):
    with self.patch_call(
        self.call.device.build_version_sdk, return_value=version_codes.KITKAT):
      with self.assertCalls((self.call.device._RunPipedShellCommand(
          'dumpsys input_method | grep mScreenOn'), self._K_SCREEN_OFF)):
        self.assertFalse(self.device.IsScreenOn())

  def testIsScreenOn_offL(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCalls((self.call.device._RunPipedShellCommand(
          'dumpsys input_method | grep mInteractive'), self._L_SCREEN_OFF)):
        self.assertFalse(self.device.IsScreenOn())

  def testIsScreenOn_noOutput(self):
    with self.patch_call(
        self.call.device.build_version_sdk,
        return_value=version_codes.LOLLIPOP):
      with self.assertCalls((self.call.device._RunPipedShellCommand(
          'dumpsys input_method | grep mInteractive'), [])):
        with self.assertRaises(device_errors.CommandFailedError):
          self.device.IsScreenOn()


class DeviecUtilsSetScreen(DeviceUtilsTest):
  @mock.patch('time.sleep', mock.Mock())
  def testSetScren_alreadySet(self):
    with self.assertCalls((self.call.device.IsScreenOn(), False)):
      self.device.SetScreen(False)

  @mock.patch('time.sleep', mock.Mock())
  def testSetScreen_on(self):
    with self.assertCalls(
        (self.call.device.IsScreenOn(), False),
        (self.call.device.SendKeyEvent(keyevent.KEYCODE_POWER), None),
        (self.call.device.IsScreenOn(), True)):
      self.device.SetScreen(True)

  @mock.patch('time.sleep', mock.Mock())
  def testSetScreen_off(self):
    with self.assertCalls(
        (self.call.device.IsScreenOn(), True),
        (self.call.device.SendKeyEvent(keyevent.KEYCODE_POWER), None),
        (self.call.device.IsScreenOn(), False)):
      self.device.SetScreen(False)

  @mock.patch('time.sleep', mock.Mock())
  def testSetScreen_slow(self):
    with self.assertCalls(
        (self.call.device.IsScreenOn(), True),
        (self.call.device.SendKeyEvent(keyevent.KEYCODE_POWER), None),
        (self.call.device.IsScreenOn(), True),
        (self.call.device.IsScreenOn(), True),
        (self.call.device.IsScreenOn(), False)):
      self.device.SetScreen(False)


class DeviecUtilsLoadCacheData(DeviceUtilsTest):
  def testInvalidJson(self):
    self.assertFalse(self.device.LoadCacheData(''))

  def testTokenMissing(self):
    with self.assertCalls(self.EnsureCacheInitialized()):
      self.assertFalse(self.device.LoadCacheData('{}'))

  def testTokenStale(self):
    with self.assertCalls(self.EnsureCacheInitialized()):
      self.assertFalse(self.device.LoadCacheData('{"token":"foo"}'))

  def testTokenMatches(self):
    with self.assertCalls(self.EnsureCacheInitialized()):
      self.assertTrue(self.device.LoadCacheData('{"token":"TOKEN"}'))

  def testDumpThenLoad(self):
    with self.assertCalls(self.EnsureCacheInitialized()):
      data = json.loads(self.device.DumpCacheData())
      data['token'] = 'TOKEN'
      self.assertTrue(self.device.LoadCacheData(json.dumps(data)))


class DeviceUtilsGetIMEITest(DeviceUtilsTest):
  def testSuccessfulDumpsys(self):
    dumpsys_output = ('Phone Subscriber Info:'
                      '  Phone Type = GSM'
                      '  Device ID = 123454321')
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '19'),
        (self.call.adb.Shell('dumpsys iphonesubinfo'), dumpsys_output)):
      self.assertEqual(self.device.GetIMEI(), '123454321')

  def testSuccessfulServiceCall(self):
    service_output = """
        Result: Parcel(\n'
          0x00000000: 00000000 0000000f 00350033 00360033 '........7.6.5.4.'
          0x00000010: 00360032 00370030 00300032 00300039 '3.2.1.0.1.2.3.4.'
          0x00000020: 00380033 00000039                   '5.6.7...        ')
    """
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '24'),
        (self.call.adb.Shell('service call iphonesubinfo 1'), service_output)):
      self.assertEqual(self.device.GetIMEI(), '765432101234567')

  def testNoIMEI(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '19'),
        (self.call.adb.Shell('dumpsys iphonesubinfo'), 'no device id')):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.GetIMEI()

  def testAdbError(self):
    with self.assertCalls(
        (self.call.device.GetProp('ro.build.version.sdk', cache=True), '24'),
        (self.call.adb.Shell('service call iphonesubinfo 1'),
         self.ShellError())):
      with self.assertRaises(device_errors.CommandFailedError):
        self.device.GetIMEI()


class DeviceUtilsChangeOwner(DeviceUtilsTest):
  def testChangeOwner(self):
    with self.assertCalls(
        (self.call.device.RunShellCommand(
            ['chown', 'user.group', '/path/to/file1', 'file2'],
            check_return=True))):
      self.device.ChangeOwner('user.group', ['/path/to/file1', 'file2'])


class DeviceUtilsChangeSecurityContext(DeviceUtilsTest):
  def testChangeSecurityContext(self):
    with self.assertCalls((self.call.device.RunShellCommand(
        ['chcon', 'u:object_r:system_data_file:s0', '/path', '/path2'],
        as_root=device_utils._FORCE_SU,
        check_return=True))):
      self.device.ChangeSecurityContext('u:object_r:system_data_file:s0',
                                        ['/path', '/path2'])


class DeviceUtilsPlaceNomediaFile(DeviceUtilsTest):
  def testPlaceNomediaFile(self):
    with self.assertCalls(
        (self.call.device.RunShellCommand(
            ['mkdir', '-p', '/sdcard/test_dir'],
            check_return=True,
            as_root=False)),
        (self.call.device.WriteFile(
            '/sdcard/test_dir/.nomedia',
            'https://crbug.com/796640',
            as_root=False))):
      self.device.PlaceNomediaFile('/sdcard/test_dir')

  def testPlaceNomediaFile_targetUser(self):
    with self.patch_call(self.call.device.target_user, return_value=10):
      with self.assertCalls(
          (self.call.device.RunShellCommand(
              ['mkdir', '-p', '/data/media/10/test_dir'],
              check_return=True,
              as_root=True)),
          (self.call.device.WriteFile(
              '/data/media/10/test_dir/.nomedia',
              'https://crbug.com/796640',
              as_root=True))):
        self.device.PlaceNomediaFile('/sdcard/test_dir')


class DeviceUtilsLocale(DeviceUtilsTest):
  def testLocaleLegacy(self):
    with self.assertCalls(
        (self.call.device.GetProp('persist.sys.locale', cache=False), ''),
        (self.call.device.GetProp('persist.sys.language', cache=False), 'en'),
        (self.call.device.GetProp('persist.sys.country', cache=False), 'US')):
      self.assertEqual(self.device.GetLocale(), ('en', 'US'))

  def testLocale(self):
    with self.assertCalls(
        (self.call.device.GetProp('persist.sys.locale', cache=False), 'en-US'),
        (self.call.device.GetProp('persist.sys.locale', cache=False),
         'en-US-sw')):
      self.assertEqual(self.device.GetLocale(), ('en', 'US'))
      self.assertEqual(self.device.GetLocale(), ('en', 'US-sw'))

  def testBadLocale(self):
    with self.assertCalls((self.call.device.GetProp(
        'persist.sys.locale', cache=False), 'en')):
      self.assertEqual(self.device.GetLocale(), ('', ''))

  def testLanguageAndCountryLegacy(self):
    with self.assertCalls(
        (self.call.device.GetProp('persist.sys.locale', cache=False), ''),
        (self.call.device.GetProp('persist.sys.language', cache=False), 'en'),
        (self.call.device.GetProp('persist.sys.country', cache=False), 'US'),
        (self.call.device.GetProp('persist.sys.locale', cache=False), ''),
        (self.call.device.GetProp('persist.sys.language', cache=False), 'en'),
        (self.call.device.GetProp('persist.sys.country', cache=False), 'US')):
      self.assertEqual(self.device.GetLanguage(), 'en')
      self.assertEqual(self.device.GetCountry(), 'US')

  def testLanguageAndCountry(self):
    with self.assertCalls(
        (self.call.device.GetProp('persist.sys.locale', cache=False), 'en-US'),
        (self.call.device.GetProp('persist.sys.locale', cache=False), 'en-US')):
      self.assertEqual(self.device.GetLanguage(), 'en')
      self.assertEqual(self.device.GetCountry(), 'US')


class IterPushableComponentsTest(unittest.TestCase):
  @classmethod
  @contextlib.contextmanager
  def sampleLayout(cls):
    Layout = collections.namedtuple('Layout', [
        'root', 'basic_file', 'symlink_file', 'symlink_dir',
        'dir_with_symlinks', 'dir_without_symlinks'
    ])

    with tempfile_ext.NamedTemporaryDirectory() as layout_root:
      dir1 = os.path.join(layout_root, 'dir1')
      os.makedirs(dir1)

      basic_file = os.path.join(dir1, 'file1.txt')
      with open(basic_file, 'w') as f:
        f.write('hello world')

      symlink = os.path.join(dir1, 'symlink.txt')
      os.symlink(basic_file, symlink)

      dir2 = os.path.join(layout_root, 'dir2')
      os.makedirs(dir2)

      with open(os.path.join(dir2, 'file2.txt'), 'w') as f:
        f.write('goodnight moon')

      symlink_dir = os.path.join(layout_root, 'dir3')
      os.symlink(dir2, symlink_dir)

      yield Layout(layout_root, basic_file, symlink, symlink_dir, dir1, dir2)

  def safeAssertItemsEqual(self, expected, actual):
    six.assertCountEqual(self, expected, actual)

  def testFile(self):
    with self.sampleLayout() as layout:
      device_path = '/sdcard/basic_file'

      expected = [(layout.basic_file, device_path, True)]
      actual = list(
          device_utils._IterPushableComponents(layout.basic_file, device_path))
      self.safeAssertItemsEqual(expected, actual)

  def testSymlinkFile(self):
    with self.sampleLayout() as layout:
      device_path = '/sdcard/basic_symlink'

      expected = [(os.path.realpath(layout.symlink_file), device_path, False)]
      actual = list(
          device_utils._IterPushableComponents(layout.symlink_file,
                                               device_path))
      self.safeAssertItemsEqual(expected, actual)

  def testDirectoryWithNoSymlink(self):
    with self.sampleLayout() as layout:
      device_path = '/sdcard/basic_directory'

      expected = [(layout.dir_without_symlinks, device_path, True)]
      actual = list(
          device_utils._IterPushableComponents(layout.dir_without_symlinks,
                                               device_path))
      self.safeAssertItemsEqual(expected, actual)

  def testDirectoryWithSymlink(self):
    with self.sampleLayout() as layout:
      device_path = '/sdcard/directory'

      expected = [
          (layout.basic_file,
           posixpath.join(device_path, os.path.basename(layout.basic_file)),
           True),
          (os.path.realpath(layout.symlink_file),
           posixpath.join(device_path, os.path.basename(layout.symlink_file)),
           False),
      ]
      actual = list(
          device_utils._IterPushableComponents(layout.dir_with_symlinks,
                                               device_path))
      self.safeAssertItemsEqual(expected, actual)

  def testSymlinkDirectory(self):
    with self.sampleLayout() as layout:
      device_path = '/sdcard/directory'

      expected = [(os.path.realpath(layout.symlink_dir), device_path, False)]
      actual = list(
          device_utils._IterPushableComponents(layout.symlink_dir, device_path))
      self.safeAssertItemsEqual(expected, actual)

  def testDirectoryWithNestedSymlink(self):
    with self.sampleLayout() as layout:
      device_path = '/sdcard/directory'

      expected = [
          (layout.dir_without_symlinks,
           posixpath.join(device_path,
                          os.path.basename(layout.dir_without_symlinks)), True),
          (layout.basic_file,
           posixpath.join(
               device_path,
               *os.path.split(os.path.relpath(layout.basic_file, layout.root))),
           True),
          (os.path.realpath(layout.symlink_file),
           posixpath.join(
               device_path,
               *os.path.split(
                   os.path.relpath(layout.symlink_file, layout.root))), False),
          (os.path.realpath(layout.symlink_dir),
           posixpath.join(
               device_path,
               *os.path.split(os.path.relpath(layout.symlink_dir,
                                              layout.root))), False),
      ]
      actual = list(
          device_utils._IterPushableComponents(layout.root, device_path))
      self.safeAssertItemsEqual(expected, actual)


class DeviceUtilsGetTracingPathTest(DeviceUtilsTest):
  def testGetTracingPath_hasDebugfs(self):
    with self.assertCalls(
        (self.call.device.RunShellCommand(['mount'], retries=0,
                                          timeout=10, check_return=True),
        ['debugfs on /sys/kernel/debug', 'proc on /proc'])):
      self.assertEqual('/sys/kernel/debug/tracing',
                       self.device.GetTracingPath())

  def testGetTracingPath_noDebugfs(self):
    with self.assertCalls(
        (self.call.device.RunShellCommand(['mount'], retries=0,
                                          timeout=10, check_return=True),
        ['proc on /proc'])):
      self.assertEqual('/sys/kernel/tracing', self.device.GetTracingPath())


if __name__ == '__main__':
  logging.getLogger().setLevel(logging.DEBUG)
  unittest.main(verbosity=2)
