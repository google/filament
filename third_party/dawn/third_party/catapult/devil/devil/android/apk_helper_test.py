#! /usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import collections
import os
import unittest

from unittest import mock

from devil.android import apk_helper
from devil.android.ndk import abis
from devil.utils import mock_calls


# pylint: disable=line-too-long
_MANIFEST_DUMP = """N: android=http://schemas.android.com/apk/res/android
  E: manifest (line=1)
    A: android:versionCode(0x0101021b)=(type 0x10)0x166de1ea
    A: android:versionName(0x0101021c)="75.0.3763.0" (Raw: "75.0.3763.0")
    A: package="org.chromium.abc" (Raw: "org.chromium.abc")
    A: split="random_split" (Raw: "random_split")
    E: uses-sdk (line=2)
      A: android:minSdkVersion(0x0101020c)=(type 0x10)0x15
      A: android:targetSdkVersion(0x01010270)=(type 0x10)0x1c
    E: uses-permission (line=2)
      A: android:name(0x01010003)="android.permission.INTERNET" (Raw: "android.permission.INTERNET")
    E: uses-permission (line=3)
      A: android:name(0x01010003)="android.permission.READ_EXTERNAL_STORAGE" (Raw: "android.permission.READ_EXTERNAL_STORAGE")
    E: uses-permission (line=4)
      A: android:name(0x01010003)="android.permission.ACCESS_FINE_LOCATION" (Raw: "android.permission.ACCESS_FINE_LOCATION")
    E: application (line=5)
      E: activity (line=6)
        A: android:name(0x01010003)="org.chromium.ActivityName" (Raw: "org.chromium.ActivityName")
        A: android:exported(0x01010010)=(type 0x12)0xffffffff
      E: service (line=7)
        A: android:name(0x01010001)="org.chromium.RandomService" (Raw: "org.chromium.RandomService")
        A: android:isolatedProcess(0x01010888)=(type 0x12)0xffffffff
      E: activity (line=173)
        A: android:name(0x01010003)=".MainActivity" (Raw: ".MainActivity")
        E: intent-filter (line=177)
          E: action (line=178)
            A: android:name(0x01010003)="android.intent.action.MAIN" (Raw: "android.intent.action.MAIN")
          E: category (line=180)
            A: android:name(0x01010003)="android.intent.category.DEFAULT" (Raw: "android.intent.category.DEFAULT")
          E: category (line=181)
            A: android:name(0x01010003)="android.intent.category.LAUNCHER" (Raw: "android.intent.category.LAUNCHER")
      E: activity-alias (line=173)
        A: android:name(0x01010003)="org.chromium.ViewActivity" (Raw: "org.chromium.ViewActivity")
        A: android:targetActivity(0x01010202)="org.chromium.ActivityName" (Raw: "org.chromium.ActivityName")
        E: intent-filter (line=191)
          E: action (line=192)
            A: android:name(0x01010003)="android.intent.action.VIEW" (Raw: "android.intent.action.VIEW")
          E: data (line=198)
            A: android:scheme(0x01010027)="http" (Raw: "http")
          E: data (line=199)
            A: android:scheme(0x01010027)="https" (Raw: "https")
      E: meta-data (line=43)
        A: android:name(0x01010003)="name1" (Raw: "name1")
        A: android:value(0x01010024)="value1" (Raw: "value1")
      E: meta-data (line=43)
        A: android:name(0x01010003)="name2" (Raw: "name2")
        A: android:value(0x01010024)="value2" (Raw: "value2")
    E: instrumentation (line=8)
      A: android:label(0x01010001)="abc" (Raw: "abc")
      A: android:name(0x01010003)="org.chromium.RandomJUnit4TestRunner" (Raw: "org.chromium.RandomJUnit4TestRunner")
      A: android:targetPackage(0x01010021)="org.chromium.random_package" (Raw:"org.chromium.random_pacakge")
      A: junit4=(type 0x12)0xffffffff (Raw: "true")
    E: instrumentation (line=9)
      A: android:label(0x01010001)="abc" (Raw: "abc")
      A: android:name(0x01010003)="org.chromium.RandomTestRunner" (Raw: "org.chromium.RandomTestRunner")
      A: android:targetPackage(0x01010021)="org.chromium.random_package" (Raw:"org.chromium.random_pacakge")
"""

_NO_ISOLATED_SERVICES = """N: android=http://schemas.android.com/apk/res/android
  E: manifest (line=1)
    A: package="org.chromium.abc" (Raw: "org.chromium.abc")
    E: application (line=5)
      E: activity (line=6)
        A: android:name(0x01010003)="org.chromium.ActivityName" (Raw: "org.chromium.ActivityName")
        A: android:exported(0x01010010)=(type 0x12)0xffffffff
      E: service (line=7)
        A: android:name(0x01010001)="org.chromium.RandomService" (Raw: "org.chromium.RandomService")
"""

_NO_SERVICES = """N: android=http://schemas.android.com/apk/res/android
  E: manifest (line=1)
    A: package="org.chromium.abc" (Raw: "org.chromium.abc")
    E: application (line=5)
      E: activity (line=6)
        A: android:name(0x01010003)="org.chromium.ActivityName" (Raw: "org.chromium.ActivityName")
        A: android:exported(0x01010010)=(type 0x12)0xffffffff
"""

_NO_APPLICATION = """N: android=http://schemas.android.com/apk/res/android
  E: manifest (line=1)
    A: package="org.chromium.abc" (Raw: "org.chromium.abc")
"""

_SINGLE_INSTRUMENTATION_MANIFEST_DUMP = """N: android=http://schemas.android.com/apk/res/android
  E: manifest (line=1)
    A: package="org.chromium.xyz" (Raw: "org.chromium.xyz")
    E: instrumentation (line=8)
      A: android:label(0x01010001)="xyz" (Raw: "xyz")
      A: android:name(0x01010003)="org.chromium.RandomTestRunner" (Raw: "org.chromium.RandomTestRunner")
      A: android:targetPackage(0x01010021)="org.chromium.random_package" (Raw:"org.chromium.random_pacakge")
"""

_SINGLE_J4_INSTRUMENTATION_MANIFEST_DUMP = """N: android=http://schemas.android.com/apk/res/android
  E: manifest (line=1)
    A: package="org.chromium.xyz" (Raw: "org.chromium.xyz")
    E: instrumentation (line=8)
      A: android:label(0x01010001)="xyz" (Raw: "xyz")
      A: android:name(0x01010003)="org.chromium.RandomJ4TestRunner" (Raw: "org.chromium.RandomJ4TestRunner")
      A: android:targetPackage(0x01010021)="org.chromium.random_package" (Raw:"org.chromium.random_pacakge")
      A: junit4=(type 0x12)0xffffffff (Raw: "true")
"""

_TARGETING_PRE_RELEASE_Q_MANIFEST_DUMP = """N: android=http://schemas.android.com/apk/res/android
  E: manifest (line=1)
    A: package="org.chromium.xyz" (Raw: "org.chromium.xyz")
    E: uses-sdk (line=2)
      A: android:minSdkVersion(0x0101020c)="Q" (Raw: "Q")
      A: android:targetSdkVersion(0x01010270)="Q" (Raw: "Q")
"""

_NO_NAMESPACE_MANIFEST_DUMP = """E: manifest (line=1)
  A: package="org.chromium.xyz" (Raw: "org.chromium.xyz")
  E: instrumentation (line=8)
    A: http://schemas.android.com/apk/res/android:label(0x01010001)="xyz" (Raw: "xyz")
    A: http://schemas.android.com/apk/res/android:name(0x01010003)="org.chromium.RandomTestRunner" (Raw: "org.chromium.RandomTestRunner")
    A: http://schemas.android.com/apk/res/android:targetPackage(0x01010021)="org.chromium.random_package" (Raw:"org.chromium.random_pacakge")
"""

_STATIC_LIBRARY_DUMP = """N: android=http://schemas.android.com/apk/res/android
  E: manifest (line=1)
    A: android:versionCode(0x0101021b)=(type 0x10)0x210cffc7
    A: package="com.google.android.trichromelibrary.debug" (Raw: "com.google.android.trichromelibrary.debug")
    E: application (line=9)
      E: static-library (line=10)
        A: android:name(0x01010003)="com.google.android.trichromelibrary.debug" (Raw: "com.google.android.trichromelibrary.debug")
        A: android:version(0x01010519)=(type 0x10)0x210cffc3
"""

# pylint: enable=line-too-long


def _MockAaptDump(manifest_dump):
  return mock.patch(
      'devil.android.sdk.aapt.Dump',
      mock.Mock(side_effect=None, return_value=manifest_dump.split('\n')))


def _MockListApkPaths(files):
  return mock.patch('devil.android.apk_helper.ApkHelper._ListApkPaths',
                    mock.Mock(side_effect=None, return_value=files))


class _MockDeviceUtils(object):
  def __init__(self):
    self.product_cpu_abi = abis.ARM_64
    self.product_cpu_abis = [abis.ARM_64, abis.ARM]
    self.pixel_density = 500
    self.build_version_sdk = 28

  def GetLocale(self):
    # pylint: disable=no-self-use
    return ('en', 'US')

  def GetFeatures(self):
    # pylint: disable=no-self-use
    return [
        'android.hardware.wifi',
        'android.hardware.nfc',
    ]


class ApkHelperTest(mock_calls.TestCase):
  def testToHelperApk(self):
    apk = apk_helper.ToHelper('abc.apk')
    self.assertTrue(isinstance(apk, apk_helper.ApkHelper))

  def testToHelperApks(self):
    apk = apk_helper.ToHelper('abc.apks')
    self.assertTrue(isinstance(apk, apk_helper.ApksHelper))

  def testToHelperApex(self):
    apex = apk_helper.ToHelper('abc.apex')
    self.assertTrue(isinstance(apex, apk_helper.ApexHelper))

  def testToHelperBundleScript(self):
    apk = apk_helper.ToHelper('abc_bundle')
    self.assertTrue(isinstance(apk, apk_helper.BundleScriptHelper))

  def testToHelperIncrementalApkFromApkHelper(self):
    apk = apk_helper.ToIncrementalHelper(apk_helper.ApkHelper('abc.apk'))
    self.assertTrue(isinstance(apk, apk_helper.IncrementalApkHelper))
    self.assertEqual(apk.path, 'abc.apk')

  def testToHelperIncrementalApkFromIncrementalApkHelper(self):
    prev_apk = apk_helper.IncrementalApkHelper('abc.apk')
    apk = apk_helper.ToIncrementalHelper(prev_apk)
    self.assertIs(prev_apk, apk)

  def testToHelperIncrementalApkFromPath(self):
    apk = apk_helper.ToIncrementalHelper('abc.apk')
    self.assertTrue(isinstance(apk, apk_helper.IncrementalApkHelper))

  def testToHelperIncrementalException(self):
    with self.assertRaises(apk_helper.ApkHelperError):
      apk_helper.ToIncrementalHelper(
          apk_helper.ToSplitHelper('abc.apk', ['a.apk', 'b.apk']))

  def testToHelperSplitApk(self):
    apk = apk_helper.ToSplitHelper('abc.apk', ['a.apk', 'b.apk'])
    self.assertTrue(isinstance(apk, apk_helper.SplitApkHelper))

  def testToHelperSplitException(self):
    with self.assertRaises(apk_helper.ApkHelperError):
      apk_helper.ToSplitHelper(
          apk_helper.ToHelper('abc.apk'), ['a.apk', 'b.apk'])

  def testGetInstrumentationName(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      with self.assertRaises(apk_helper.ApkHelperError):
        helper.GetInstrumentationName()

  def testGetActivityName(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual(helper.GetActivityName(),
                       'org.chromium.abc.MainActivity')

  def testGetViewActivityName(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual(helper.GetViewActivityName(),
                       'org.chromium.ViewActivity')

  def testGetAllInstrumentations(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      all_instrumentations = helper.GetAllInstrumentations()
      self.assertEqual(len(all_instrumentations), 2)
      self.assertEqual(all_instrumentations[0]['android:name'],
                       'org.chromium.RandomJUnit4TestRunner')
      self.assertEqual(all_instrumentations[1]['android:name'],
                       'org.chromium.RandomTestRunner')

  def testGetPackageName(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual(helper.GetPackageName(), 'org.chromium.abc')

  def testGetPermissions(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      all_permissions = helper.GetPermissions()
      self.assertEqual(len(all_permissions), 3)
      self.assertTrue('android.permission.INTERNET' in all_permissions)
      self.assertTrue(
          'android.permission.READ_EXTERNAL_STORAGE' in all_permissions)
      self.assertTrue(
          'android.permission.ACCESS_FINE_LOCATION' in all_permissions)

  def testGetSplitName(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual(helper.GetSplitName(), 'random_split')

  def testHasIsolatedProcesses_noApplication(self):
    with _MockAaptDump(_NO_APPLICATION):
      helper = apk_helper.ApkHelper('')
      self.assertFalse(helper.HasIsolatedProcesses())

  def testHasIsolatedProcesses_noServices(self):
    with _MockAaptDump(_NO_SERVICES):
      helper = apk_helper.ApkHelper('')
      self.assertFalse(helper.HasIsolatedProcesses())

  def testHasIsolatedProcesses_oneNotIsolatedProcess(self):
    with _MockAaptDump(_NO_ISOLATED_SERVICES):
      helper = apk_helper.ApkHelper('')
      self.assertFalse(helper.HasIsolatedProcesses())

  def testHasIsolatedProcesses_oneIsolatedProcess(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertTrue(helper.HasIsolatedProcesses())

  def testGetSingleInstrumentationName(self):
    with _MockAaptDump(_SINGLE_INSTRUMENTATION_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual('org.chromium.RandomTestRunner',
                       helper.GetInstrumentationName())

  def testGetSingleJUnit4InstrumentationName(self):
    with _MockAaptDump(_SINGLE_J4_INSTRUMENTATION_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual('org.chromium.RandomJ4TestRunner',
                       helper.GetInstrumentationName())

  def testGetAllMetadata(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual([('name1', 'value1'), ('name2', 'value2')],
                       helper.GetAllMetadata())

  def testGetLibraryVersion(self):
    with _MockAaptDump(_STATIC_LIBRARY_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual(554500035, helper.GetLibraryVersion())

  def testGetLibraryVersion_normalApk(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual(None, helper.GetLibraryVersion())

  def testGetVersionCode(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual(376300010, helper.GetVersionCode())

  def testGetVersionName(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual('75.0.3763.0', helper.GetVersionName())

  def testGetMinSdkVersion_integerValue(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual('21', helper.GetMinSdkVersion())

  def testGetMinSdkVersion_stringValue(self):
    with _MockAaptDump(_TARGETING_PRE_RELEASE_Q_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual('Q', helper.GetMinSdkVersion())

  def testGetTargetSdkVersion_integerValue(self):
    with _MockAaptDump(_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual('28', helper.GetTargetSdkVersion())

  def testGetTargetSdkVersion_stringValue(self):
    with _MockAaptDump(_TARGETING_PRE_RELEASE_Q_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual('Q', helper.GetTargetSdkVersion())

  def testGetSingleInstrumentationName_strippedNamespaces(self):
    with _MockAaptDump(_NO_NAMESPACE_MANIFEST_DUMP):
      helper = apk_helper.ApkHelper('')
      self.assertEqual('org.chromium.RandomTestRunner',
                       helper.GetInstrumentationName())

  def testSetExtraApkPaths(self):
    apk = apk_helper.ToIncrementalHelper('abc.apk')
    apk.SetExtraApkPaths(['extra.apk'])
    with apk.GetApkPaths(_MockDeviceUtils()) as apk_paths:
      self.assertEqual(apk_paths, ['abc.apk', 'extra.apk'])

  def testGetArchitectures(self):
    AbiPair = collections.namedtuple('AbiPair', ['abi32bit', 'abi64bit'])
    for abi_pair in [
        AbiPair('lib/' + abis.ARM, 'lib/' + abis.ARM_64),
        AbiPair('lib/' + abis.X86, 'lib/' + abis.X86_64)
    ]:
      with _MockListApkPaths([abi_pair.abi32bit]):
        helper = apk_helper.ApkHelper('')
        self.assertEqual(
            set([
                os.path.basename(abi_pair.abi32bit),
                os.path.basename(abi_pair.abi64bit)
            ]), set(helper.GetAbis()))
      with _MockListApkPaths([abi_pair.abi32bit, abi_pair.abi64bit]):
        helper = apk_helper.ApkHelper('')
        self.assertEqual(
            set([
                os.path.basename(abi_pair.abi32bit),
                os.path.basename(abi_pair.abi64bit)
            ]), set(helper.GetAbis()))
      with _MockListApkPaths([abi_pair.abi64bit]):
        helper = apk_helper.ApkHelper('')
        self.assertEqual(set([os.path.basename(abi_pair.abi64bit)]),
                         set(helper.GetAbis()))

  def testGetSplitsApk(self):
    apk = apk_helper.ToHelper('abc.apk')
    with apk.GetApkPaths(_MockDeviceUtils()) as apk_paths:
      self.assertEqual(apk_paths, ['abc.apk'])

  def testGetSplitsApkModulesException(self):
    apk = apk_helper.ToHelper('abc.apk')
    with self.assertRaises(apk_helper.ApkHelperError):
      apk.GetApkPaths(None, modules=['a'])

  def testGetSplitsApks(self):
    apk = apk_helper.ToHelper('abc.apks')
    with self.assertCalls(
        (mock.call.tempfile.mkdtemp(),
         '/tmp'),
        (mock.call.devil.android.sdk.bundletool.ExtractApks(
            '/tmp', 'abc.apks', ['arm64-v8a', 'armeabi-v7a'], [('en', 'US')],
            ['android.hardware.wifi', 'android.hardware.nfc'], 500, 28, None)),
        (mock.call.os.listdir('/tmp'), ['base-master.apk', 'foo-master.apk']),
        (mock.call.shutil.rmtree('/tmp')),
    ),\
    apk.GetApkPaths(_MockDeviceUtils()) as apk_paths:
      self.assertEqual(apk_paths,
                       ['/tmp/base-master.apk', '/tmp/foo-master.apk'])

  def testGetSplitsApksWithModules(self):
    apk = apk_helper.ToHelper('abc.apks')
    with self.assertCalls(
        (mock.call.tempfile.mkdtemp(),
         '/tmp'),
        (mock.call.devil.android.sdk.bundletool.ExtractApks(
            '/tmp', 'abc.apks', ['arm64-v8a', 'armeabi-v7a'], [('en', 'US')],
            ['android.hardware.wifi', 'android.hardware.nfc'], 500, 28,
            ['bar'])),
        (mock.call.os.listdir('/tmp'),
         ['base-master.apk', 'foo-master.apk', 'bar-master.apk']),
        (mock.call.shutil.rmtree('/tmp')),
    ),\
    apk.GetApkPaths(_MockDeviceUtils(), ['bar']) as apk_paths:
      self.assertEqual(apk_paths, [
          '/tmp/base-master.apk', '/tmp/foo-master.apk', '/tmp/bar-master.apk'
      ])

  def testGetSplitsApksWithAdditionalLocales(self):
    apk = apk_helper.ToHelper('abc.apks')
    with self.assertCalls(
        (mock.call.tempfile.mkdtemp(),
         '/tmp'),
        (mock.call.devil.android.sdk.bundletool.ExtractApks(
            '/tmp', 'abc.apks', ['arm64-v8a', 'armeabi-v7a'],
            [('en', 'US'), ('es', 'ES'), ('fr', 'CA')],
            ['android.hardware.wifi', 'android.hardware.nfc'], 500, 28, None)),
        (mock.call.os.listdir('/tmp'),
         ['base-master.apk', 'base-es.apk', 'base-fr.apk']),
        (mock.call.shutil.rmtree('/tmp')),
    ),\
        apk.GetApkPaths(_MockDeviceUtils(),
                        additional_locales=['es-ES', 'fr-CA']) as apk_paths:
      self.assertEqual(
          apk_paths,
          ['/tmp/base-master.apk', '/tmp/base-es.apk', '/tmp/base-fr.apk'])

  def testGetSplitsApksWithAdditionalLocalesIncorrectFormat(self):
    apk = apk_helper.ToHelper('abc.apks')
    with self.assertRaises(apk_helper.ApkHelperError):
      apk.GetApkPaths(_MockDeviceUtils(), additional_locales=['es'])

  def testGetSplitsSplitApk(self):
    apk = apk_helper.ToSplitHelper('base.apk',
                                   ['split1.apk', 'split2.apk', 'split3.apk'])
    device = _MockDeviceUtils()
    with self.assertCalls(
        (mock.call.devil.android.sdk.split_select.SelectSplits(
            device,
            'base.apk', ['split1.apk', 'split2.apk', 'split3.apk'],
            allow_cached_props=False), ['split2.apk'])),\
      apk.GetApkPaths(device) as apk_paths:
      self.assertEqual(apk_paths, ['base.apk', 'split2.apk'])

  def testGetSplitsBundleScript(self):
    apk = apk_helper.ToHelper('abc_bundle')
    device = _MockDeviceUtils()
    with self.assertCalls(
        (mock.call.tempfile.mkstemp(suffix='.apks'), (0, '/tmp/abc.apks')),
        (mock.call.devil.utils.cmd_helper.GetCmdStatusOutputAndError([
            'abc_bundle', 'build-bundle-apks', '--output-apks', '/tmp/abc.apks'
        ]), (0, '', '')),
        (mock.call.tempfile.mkdtemp(), '/tmp2'),
        (mock.call.devil.android.sdk.bundletool.ExtractApks(
            '/tmp2', '/tmp/abc.apks', ['arm64-v8a', 'armeabi-v7a'],
            [('en', 'US')], ['android.hardware.wifi', 'android.hardware.nfc'],
            500, 28, ['bar'])),
        (mock.call.os.listdir('/tmp2'), ['base-master.apk', 'bar-master.apk']),
        (mock.call.os.path.isfile('/tmp/abc.apks'), True),
        (mock.call.os.remove('/tmp/abc.apks')),
        (mock.call.os.path.isfile('/tmp2'), False),
        (mock.call.os.path.isdir('/tmp2'), True),
        (mock.call.shutil.rmtree('/tmp2')),
    ),\
    apk.GetApkPaths(device, modules=['bar']) as apk_paths:
      self.assertEqual(apk_paths,
                       ['/tmp2/base-master.apk', '/tmp2/bar-master.apk'])


if __name__ == '__main__':
  unittest.main(verbosity=2)
