# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import posixpath
import unittest
from unittest import mock

from pyfakefs import fake_filesystem_unittest
from py_utils import tempfile_ext

from telemetry.core import android_platform
from telemetry.core import exceptions
from telemetry import decorators
from telemetry.internal.backends import android_browser_backend_settings
from telemetry.internal.backends.chrome import android_browser_finder
from telemetry.internal.browser import browser_finder
from telemetry.internal.platform import android_platform_backend
from telemetry.internal.util import binary_manager
from telemetry.testing import options_for_unittests


def FakeFetchPath(dependency, os_name, arch, os_version=None):
  return os.path.join('dependency_dir', dependency,
                      '%s_%s_%s.apk' % (os_name, os_version, arch))


class AndroidBrowserFinderTest(fake_filesystem_unittest.TestCase):
  def setUp(self):
    self.finder_options = options_for_unittests.GetCopy()
    # Mock out what's needed for testing with exact APKs
    self.setUpPyfakefs()
    self._fetch_path_patcher = mock.patch(
        'telemetry.internal.backends.chrome.android_browser_finder.binary_manager.FetchPath',  # pylint: disable=line-too-long
        FakeFetchPath)
    self._fetch_path_mock = self._fetch_path_patcher.start()
    self._get_package_name_patcher = mock.patch(
        'devil.android.apk_helper.GetPackageName')
    self._get_package_name_mock = self._get_package_name_patcher.start()
    self.fake_platform = mock.Mock(spec=android_platform.AndroidPlatform)
    self.fake_platform.CanLaunchApplication.return_value = True
    self.fake_platform._platform_backend = mock.Mock(
        spec=android_platform_backend.AndroidPlatformBackend)
    device = self.fake_platform._platform_backend.device
    device.build_description = 'some L device'
    device.build_version_sdk = 21
    self.fake_platform.GetOSVersionName.return_value = 'L23ds5'
    self.fake_platform.GetArchName.return_value = 'armeabi-v7a'
    # The android_browser_finder converts the os version name to 'k' or 'l'
    self.expected_reference_build = FakeFetchPath(
        'chrome_stable', 'android', 'armeabi-v7a', 'l')

  def tearDown(self):
    self.tearDownPyfakefs()
    self._get_package_name_patcher.stop()
    self._fetch_path_patcher.stop()

  def testNoPlatformReturnsEmptyList(self):
    fake_platform = None
    possible_browsers = android_browser_finder._FindAllPossibleBrowsers(
        self.finder_options, fake_platform)
    self.assertEqual([], possible_browsers)

  def testCanLaunchAlwaysTrueReturnsAllExceptExactAndReference(self):
    self.finder_options.browser_type = 'any'
    all_types = set(
        android_browser_finder.FindAllBrowserTypes())
    expected_types = all_types - {'exact', 'reference'}
    possible_browsers = android_browser_finder._FindAllPossibleBrowsers(
        self.finder_options, self.fake_platform)
    self.assertEqual(
        expected_types,
        {b.browser_type for b in possible_browsers})

  def testCanLaunchAlwaysTrueReturnsAllExceptExact(self):
    self.finder_options.browser_type = 'any'
    self.fs.CreateFile(self.expected_reference_build)
    all_types = set(
        android_browser_finder.FindAllBrowserTypes())
    expected_types = all_types - {'exact'}
    possible_browsers = android_browser_finder._FindAllPossibleBrowsers(
        self.finder_options, self.fake_platform)
    self.assertEqual(
        expected_types,
        {b.browser_type for b in possible_browsers})

  def testCanLaunchAlwaysTrueWithExactApkReturnsAll(self):
    self.fs.CreateFile(
        '/foo/ContentShell.apk')
    self.fs.CreateFile(self.expected_reference_build)
    self.finder_options.browser_type = 'any'
    self.finder_options.browser_executable = '/foo/ContentShell.apk'
    self._get_package_name_mock.return_value = 'org.chromium.content_shell_apk'

    expected_types = set(
        android_browser_finder.FindAllBrowserTypes())
    possible_browsers = android_browser_finder._FindAllPossibleBrowsers(
        self.finder_options, self.fake_platform)
    self.assertEqual(
        expected_types,
        {b.browser_type for b in possible_browsers})

  def testErrorWithUnknownExactApk(self):
    self.fs.CreateFile(
        '/foo/ContentShell.apk')
    self.finder_options.browser_executable = '/foo/ContentShell.apk'
    self._get_package_name_mock.return_value = 'org.unknown.app'

    self.assertRaises(Exception,
                      android_browser_finder._FindAllPossibleBrowsers,
                      self.finder_options, self.fake_platform)

  def testErrorWithNonExistantExactApk(self):
    self.finder_options.browser_executable = '/foo/ContentShell.apk'
    self._get_package_name_mock.return_value = 'org.chromium.content_shell_apk'

    self.assertRaises(Exception,
                      android_browser_finder._FindAllPossibleBrowsers,
                      self.finder_options, self.fake_platform)

  def testErrorWithUnrecognizedApkName(self):
    self.fs.CreateFile(
        '/foo/unknown.apk')
    self.finder_options.browser_executable = '/foo/unknown.apk'
    self._get_package_name_mock.return_value = 'org.foo.bar'

    with self.assertRaises(exceptions.UnknownPackageError):
      android_browser_finder._FindAllPossibleBrowsers(
          self.finder_options, self.fake_platform)

  def testCanLaunchExactWithUnrecognizedApkNameButKnownPackageName(self):
    self.fs.CreateFile(
        '/foo/MyFooBrowser.apk')
    self._get_package_name_mock.return_value = 'org.chromium.chrome'
    self.finder_options.browser_executable = '/foo/MyFooBrowser.apk'

    possible_browsers = android_browser_finder._FindAllPossibleBrowsers(
        self.finder_options, self.fake_platform)
    self.assertIn('exact', [b.browser_type for b in possible_browsers])

  def testNoErrorWithMissingReferenceBuild(self):
    possible_browsers = android_browser_finder._FindAllPossibleBrowsers(
        self.finder_options, self.fake_platform)
    self.assertNotIn('reference', [b.browser_type for b in possible_browsers])

  def testNoErrorWithReferenceBuildCloudStorageError(self):
    with mock.patch(
        'telemetry.internal.backends.chrome.android_browser_finder.binary_manager.FetchPath',  # pylint: disable=line-too-long
        side_effect=binary_manager.CloudStorageError):
      possible_browsers = android_browser_finder._FindAllPossibleBrowsers(
          self.finder_options, self.fake_platform)
    self.assertNotIn('reference', [b.browser_type for b in possible_browsers])

  def testNoErrorWithReferenceBuildNoPathFoundError(self):
    self._fetch_path_mock.side_effect = binary_manager.NoPathFoundError
    possible_browsers = android_browser_finder._FindAllPossibleBrowsers(
        self.finder_options, self.fake_platform)
    self.assertNotIn('reference', [b.browser_type for b in possible_browsers])

  def testWebViewBrowserReturned(self):
    self.finder_options.browser_type = 'android-webview'
    possible_browsers = android_browser_finder._FindAllPossibleBrowsers(
        self.finder_options, self.fake_platform)
    self.assertEqual(possible_browsers[0].target_os, 'android_webview')

  def testCanPossiblyHandlePath(self):
    self.assertTrue(android_browser_finder._CanPossiblyHandlePath('foo.apk'))
    self.assertTrue(android_browser_finder._CanPossiblyHandlePath('foo_bundle'))
    self.assertFalse(android_browser_finder._CanPossiblyHandlePath('f.bundle'))
    self.assertFalse(android_browser_finder._CanPossiblyHandlePath(''))
    self.assertFalse(android_browser_finder._CanPossiblyHandlePath('fooaab'))

  def testModulesPassedToInstallApplicationForBundle(self):
    self.finder_options.modules_to_install = ['base']
    self.fs.CreateFile('foo_bundle')
    possible_browser = android_browser_finder.PossibleAndroidBrowser(
        'android-chromium-bundle', self.finder_options, self.fake_platform,
        android_browser_backend_settings.ANDROID_CHROMIUM_BUNDLE, 'foo_bundle')
    with mock.patch.object(self.fake_platform._platform_backend,
                           'InstallApplication') as m:
      possible_browser.UpdateExecutableIfNeeded()
      m.assert_called_with('foo_bundle', modules={'base'})

  def testAndroid_Not_WebviewTagInTypExpectationsTags(self):
    self.finder_options.modules_to_install = ['base']
    self.fs.CreateFile('foo_bundle')
    with mock.patch.object(self.fake_platform,
                           'GetTypExpectationsTags', return_value=['android']):
      possible_browser = android_browser_finder.PossibleAndroidBrowser(
          'android-chromium-bundle', self.finder_options, self.fake_platform,
          android_browser_backend_settings.ANDROID_CHROMIUM_BUNDLE,
          'foo_bundle')
      self.assertIn('android-not-webview',
                    possible_browser.GetTypExpectationsTags())
      self.assertIn('android',
                    possible_browser.GetTypExpectationsTags())

  def testAndroidWebviewTagInTypExpectationsTags(self):
    self.finder_options.modules_to_install = ['base']
    self.fs.CreateFile('foo_bundle')
    with mock.patch.object(self.fake_platform,
                           'GetTypExpectationsTags', return_value=['android']):
      possible_browser = android_browser_finder.PossibleAndroidBrowser(
          'android-webview-google', self.finder_options, self.fake_platform,
          android_browser_backend_settings.ANDROID_WEBVIEW_GOOGLE,
          'foo_bundle')
      self.assertIn('android-webview',
                    possible_browser.GetTypExpectationsTags())
      self.assertIn('android',
                    possible_browser.GetTypExpectationsTags())


def _MockPossibleBrowser(modified_at):
  m = mock.Mock(spec=android_browser_finder.PossibleAndroidBrowser)
  m.last_modification_time = modified_at
  return m


class SelectDefaultBrowserTest(unittest.TestCase):
  def testEmptyListGivesNone(self):
    self.assertIsNone(android_browser_finder.SelectDefaultBrowser([]))

  def testSinglePossibleReturnsSame(self):
    possible_browsers = [_MockPossibleBrowser(modified_at=1)]
    self.assertIs(
        possible_browsers[0],
        android_browser_finder.SelectDefaultBrowser(possible_browsers))

  def testListGivesNewest(self):
    possible_browsers = [
        _MockPossibleBrowser(modified_at=2),
        _MockPossibleBrowser(modified_at=3),  # newest
        _MockPossibleBrowser(modified_at=1),
        ]
    self.assertIs(
        possible_browsers[1],
        android_browser_finder.SelectDefaultBrowser(possible_browsers))


class SetUpProfileBrowserTest(unittest.TestCase):

  @decorators.Enabled('android')
  def testPushEmptyProfile(self):
    finder_options = options_for_unittests.GetCopy()
    finder_options.browser_options.profile_dir = None
    browser_to_create = browser_finder.FindBrowser(finder_options)
    profile_dir = browser_to_create.profile_directory
    device = browser_to_create._platform_backend.device

    # Depending on Android version, the profile directory may have a 'lib'
    # folder. This folder must not be deleted when we push an empty profile.
    # Remember the existence of this folder so that we can check for accidental
    # deletion later.
    has_lib_dir = 'lib' in device.ListDirectory(profile_dir, as_root=True)

    try:
      # SetUpEnvironment will call RemoveProfile on the device, due to the fact
      # that there is no input profile directory in BrowserOptions.
      browser_to_create.SetUpEnvironment(finder_options.browser_options)

      # On some devices, "lib" is created after installing the browser,
      # and pushing / removing the profile should never modify it.
      profile_paths = device.ListDirectory(profile_dir, as_root=True)
      expected_paths = ['lib'] if has_lib_dir else []
      self.assertEqual(expected_paths, profile_paths)

    finally:
      browser_to_create.CleanUpEnvironment()

  @decorators.Enabled('android')
  def testPushDefaultProfileDir(self):
    # Add a few files and directories to a temp directory, and ensure they are
    # copied to the device.
    with tempfile_ext.NamedTemporaryDirectory() as tempdir:
      foo_path = os.path.join(tempdir, 'foo')
      with open(foo_path, 'w') as f:
        f.write('foo_data')

      bar_path = os.path.join(tempdir, 'path', 'to', 'bar')
      os.makedirs(os.path.dirname(bar_path))
      with open(bar_path, 'w') as f:
        f.write('bar_data')

      expected_profile_paths = ['foo', posixpath.join('path', 'to', 'bar')]

      finder_options = options_for_unittests.GetCopy()
      finder_options.browser_options.profile_dir = tempdir
      browser_to_create = browser_finder.FindBrowser(finder_options)

      # SetUpEnvironment will end up calling PushProfile
      try:
        browser_to_create.SetUpEnvironment(finder_options.browser_options)

        profile_dir = browser_to_create.profile_directory
        device = browser_to_create._platform_backend.device

        absolute_expected_profile_paths = [
            posixpath.join(profile_dir, path)
            for path in expected_profile_paths]
        device = browser_to_create._platform_backend.device
        self.assertTrue(device.PathExists(absolute_expected_profile_paths),
                        absolute_expected_profile_paths)
      finally:
        browser_to_create.CleanUpEnvironment()

  @decorators.Enabled('android')
  def testPushDefaultProfileFiles(self):
    # Add a few files and directories to a temp directory, and ensure they are
    # copied to the device.
    with tempfile_ext.NamedTemporaryDirectory() as tempdir:
      foo_path = os.path.join(tempdir, 'foo')
      with open(foo_path, 'w') as f:
        f.write('foo_data')

      bar_path = os.path.join(tempdir, 'path', 'to', 'bar')
      os.makedirs(os.path.dirname(bar_path))
      with open(bar_path, 'w') as f:
        f.write('bar_data')

      finder_options = options_for_unittests.GetCopy()
      finder_options.browser_options.profile_files_to_copy = [
          (foo_path, 'foo'),
          (bar_path, posixpath.join('path', 'to', 'bar'))]
      browser_to_create = browser_finder.FindBrowser(finder_options)

      # SetUpEnvironment will end up calling PushProfile
      try:
        browser_to_create.SetUpEnvironment(finder_options.browser_options)

        profile_dir = browser_to_create.profile_directory
        device = browser_to_create._platform_backend.device

        absolute_expected_profile_paths = [
            posixpath.join(profile_dir, path)
            for _, path
            in finder_options.browser_options.profile_files_to_copy]
        device = browser_to_create._platform_backend.device
        self.assertTrue(device.PathExists(absolute_expected_profile_paths),
                        absolute_expected_profile_paths)
      finally:
        browser_to_create.CleanUpEnvironment()
