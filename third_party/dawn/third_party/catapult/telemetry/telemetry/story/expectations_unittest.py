# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
import six


from telemetry.core import os_version as os_version_module
from telemetry.story import expectations
from telemetry.testing import fakes


class MockState():
  def __init__(self):
    self.platform = fakes.FakePlatform()


class MockStory():
  def __init__(self, name):
    self._name = name

  @property
  def name(self):
    return self._name


class MockStorySet():
  def __init__(self, stories):
    self._stories = stories

  @property
  def stories(self):
    return self._stories

class MockBrowserFinderOptions():
  def __init__(self):
    self._browser_type = None

  @property
  def browser_type(self):
    return self._browser_type

  @browser_type.setter
  def browser_type(self, t):
    assert isinstance(t, six.string_types)
    self._browser_type = t


class TestConditionTest(unittest.TestCase):
  def setUp(self):
    self._platform = fakes.FakePlatform()
    self._finder_options = MockBrowserFinderOptions()

  def testAllAlwaysReturnsTrue(self):
    self.assertTrue(
        expectations.ALL.ShouldDisable(self._platform, self._finder_options))

  def testAllWinReturnsTrueOnWindows(self):
    self._platform.SetOSName('win')
    self.assertTrue(
        expectations.ALL_WIN.ShouldDisable(self._platform,
                                           self._finder_options))

  def testAllWinReturnsFalseOnOthers(self):
    self._platform.SetOSName('not_windows')
    self.assertFalse(
        expectations.ALL_WIN.ShouldDisable(self._platform,
                                           self._finder_options))

  def testAllLinuxReturnsTrueOnLinux(self):
    self._platform.SetOSName('linux')
    self.assertTrue(expectations.ALL_LINUX.ShouldDisable(self._platform,
                                                         self._finder_options))

  def testAllLinuxReturnsFalseOnOthers(self):
    self._platform.SetOSName('not_linux')
    self.assertFalse(expectations.ALL_LINUX.ShouldDisable(self._platform,
                                                          self._finder_options))

  def testAllMacReturnsTrueOnMac(self):
    self._platform.SetOSName('mac')
    self.assertTrue(expectations.ALL_MAC.ShouldDisable(self._platform,
                                                       self._finder_options))

  def testAllMacReturnsFalseOnOthers(self):
    self._platform.SetOSName('not_mac')
    self.assertFalse(expectations.ALL_MAC.ShouldDisable(self._platform,
                                                        self._finder_options))

  def testAllChromeOSReturnsTrueOnChromeOS(self):
    self._platform.SetOSName('chromeos')
    self.assertTrue(expectations.ALL_CHROMEOS.ShouldDisable(
        self._platform, self._finder_options))

  def testAllChromeOSReturnsFalseOnOthers(self):
    self._platform.SetOSName('not_chromeos')
    self.assertFalse(expectations.ALL_CHROMEOS.ShouldDisable(
        self._platform, self._finder_options))

  def testAllAndroidReturnsTrueOnAndroid(self):
    self._platform.SetOSName('android')
    self.assertTrue(
        expectations.ALL_ANDROID.ShouldDisable(self._platform,
                                               self._finder_options))

  def testAllAndroidReturnsFalseOnOthers(self):
    self._platform.SetOSName('not_android')
    self.assertFalse(
        expectations.ALL_ANDROID.ShouldDisable(self._platform,
                                               self._finder_options))

  def testAllDesktopReturnsFalseOnNonDesktop(self):
    false_platforms = ['android']
    for plat in false_platforms:
      self._platform.SetOSName(plat)
      self.assertFalse(
          expectations.ALL_DESKTOP.ShouldDisable(self._platform,
                                                 self._finder_options))

  def testAllDesktopReturnsTrueOnDesktop(self):
    true_platforms = ['win', 'mac', 'linux', 'chromeos']
    for plat in true_platforms:
      self._platform.SetOSName(plat)
      self.assertTrue(
          expectations.ALL_DESKTOP.ShouldDisable(self._platform,
                                                 self._finder_options))

  def testAllMobileReturnsFalseOnNonMobile(self):
    false_platforms = ['win', 'mac', 'linux', 'chromeos']
    for plat in false_platforms:
      self._platform.SetOSName(plat)
      self.assertFalse(
          expectations.ALL_MOBILE.ShouldDisable(self._platform,
                                                self._finder_options))

  def testAllMobileReturnsTrueOnMobile(self):
    true_platforms = ['android']
    for plat in true_platforms:
      self._platform.SetOSName(plat)
      self.assertTrue(
          expectations.ALL_MOBILE.ShouldDisable(self._platform,
                                                self._finder_options))

  def testAndroidNexus5ReturnsFalseOnNotAndroid(self):
    self._platform.SetOSName('not_android')
    self.assertFalse(
        expectations.ANDROID_NEXUS5.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidNexus5XReturnsFalseOnNotAndroid(self):
    self._platform.SetOSName('not_android')
    self.assertFalse(
        expectations.ANDROID_NEXUS5X.ShouldDisable(self._platform,
                                                   self._finder_options))

  def testAndroidNexus6ReturnsFalseOnNotAndroid(self):
    self._platform.SetOSName('not_android')
    self.assertFalse(
        expectations.ANDROID_NEXUS6.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidNexus6PReturnsFalseOnNotAndroid(self):
    self._platform.SetOSName('not_android')
    self.assertFalse(
        expectations.ANDROID_NEXUS6P.ShouldDisable(self._platform,
                                                   self._finder_options))

  def testAndroidNexus7ReturnsFalseOnNotAndroid(self):
    self._platform.SetOSName('not_android')
    self.assertFalse(
        expectations.ANDROID_NEXUS7.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidCherryMobileReturnsFalseOnNotAndroid(self):
    self._platform.SetOSName('not_android')
    self.assertFalse(
        expectations.ANDROID_ONE.ShouldDisable(self._platform,
                                               self._finder_options))

  def testAndroidSvelteReturnsFalseOnNotAndroid(self):
    self._platform.SetOSName('not_android')
    self.assertFalse(
        expectations.ANDROID_SVELTE.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidNexus5ReturnsFalseOnAndroidNotNexus5(self):
    self._platform.SetOSName('android')
    self.assertFalse(
        expectations.ANDROID_NEXUS5.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidNexus5XReturnsFalseOnAndroidNotNexus5X(self):
    self._platform.SetOSName('android')
    self.assertFalse(
        expectations.ANDROID_NEXUS5X.ShouldDisable(self._platform,
                                                   self._finder_options))

  def testAndroidNexus5ReturnsFalseOnAndroidNexus5X(self):
    self._platform.SetOSName('android')
    self._platform.SetDeviceTypeName('Nexus 5X')
    self.assertFalse(
        expectations.ANDROID_NEXUS5.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidNexus6ReturnsFalseOnAndroidNotNexus6(self):
    self._platform.SetOSName('android')
    self.assertFalse(
        expectations.ANDROID_NEXUS6.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidNexus6ReturnsFalseOnAndroidNexus6P(self):
    self._platform.SetOSName('android')
    self._platform.SetDeviceTypeName('Nexus 6P')
    self.assertFalse(
        expectations.ANDROID_NEXUS6.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidNexus6PReturnsFalseOnAndroidNotNexus6P(self):
    self._platform.SetOSName('android')
    self.assertFalse(
        expectations.ANDROID_NEXUS6P.ShouldDisable(self._platform,
                                                   self._finder_options))

  def testAndroidNexus7ReturnsFalseOnAndroidNotNexus7(self):
    self._platform.SetOSName('android')
    self.assertFalse(
        expectations.ANDROID_NEXUS7.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidCherryMobileReturnsFalseOnAndroidNotCherryMobile(self):
    self._platform.SetOSName('android')
    self.assertFalse(
        expectations.ANDROID_ONE.ShouldDisable(self._platform,
                                               self._finder_options))

  def testAndroidSvelteReturnsFalseOnAndroidNotSvelte(self):
    self._platform.SetOSName('android')
    self.assertFalse(
        expectations.ANDROID_SVELTE.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidNexus5ReturnsTrueOnAndroidNexus5(self):
    self._platform.SetOSName('android')
    self._platform.SetDeviceTypeName('Nexus 5')
    self.assertTrue(
        expectations.ANDROID_NEXUS5.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidNexus5XReturnsTrueOnAndroidNexus5X(self):
    self._platform.SetOSName('android')
    self._platform.SetDeviceTypeName('Nexus 5X')
    self.assertTrue(
        expectations.ANDROID_NEXUS5X.ShouldDisable(self._platform,
                                                   self._finder_options))

  def testAndroidNexus6ReturnsTrueOnAndroidNexus6(self):
    self._platform.SetOSName('android')
    self._platform.SetDeviceTypeName('Nexus 6')
    self.assertTrue(
        expectations.ANDROID_NEXUS6.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidNexus6PReturnsTrueOnAndroidNexus6P(self):
    self._platform.SetOSName('android')
    self._platform.SetDeviceTypeName('Nexus 6P')
    self.assertTrue(
        expectations.ANDROID_NEXUS6P.ShouldDisable(self._platform,
                                                   self._finder_options))

  def testAndroidNexus7ReturnsTrueOnAndroidNexus7(self):
    self._platform.SetOSName('android')
    self._platform.SetDeviceTypeName('Nexus 7')
    self.assertTrue(
        expectations.ANDROID_NEXUS7.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidCherryMobileReturnsTrueOnAndroidCherryMobile(self):
    self._platform.SetOSName('android')
    self._platform.SetDeviceTypeName('W6210')
    self.assertTrue(
        expectations.ANDROID_ONE.ShouldDisable(self._platform,
                                               self._finder_options))

  def testAndroidSvelteReturnsTrueOnAndroidSvelte(self):
    self._platform.SetOSName('android')
    self._platform.SetIsSvelte(True)
    self.assertTrue(
        expectations.ANDROID_SVELTE.ShouldDisable(self._platform,
                                                  self._finder_options))

  def testAndroidWebviewReturnsTrueOnAndroidWebview(self):
    self._platform.SetOSName('android')
    self._platform.SetIsAosp(True)
    self._finder_options.browser_type = 'android-webview'
    self.assertTrue(
        expectations.ANDROID_WEBVIEW.ShouldDisable(self._platform,
                                                   self._finder_options))

  def testAndroidWebviewReturnsTrueOnAndroidWebviewGoogle(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android-webview-google'
    self.assertTrue(
        expectations.ANDROID_WEBVIEW.ShouldDisable(self._platform,
                                                   self._finder_options))

  def testAndroidWebviewReturnsFalseOnAndroidNotWebview(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android-chrome'
    self.assertFalse(
        expectations.ANDROID_WEBVIEW.ShouldDisable(self._platform,
                                                   self._finder_options))

  def testAndroidWebviewReturnsFalseOnNotAndroid(self):
    self._platform.SetOSName('not_android')
    self.assertFalse(
        expectations.ANDROID_WEBVIEW.ShouldDisable(self._platform,
                                                   self._finder_options))

  def testAndroidNotWebviewReturnsTrueOnAndroidNotWebview(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android'
    self.assertTrue(
        expectations.ANDROID_NOT_WEBVIEW.ShouldDisable(self._platform,
                                                       self._finder_options))

  def testAndroidNotWebviewReturnsFalseOnAndroidWebview(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android-webview'
    self.assertFalse(
        expectations.ANDROID_NOT_WEBVIEW.ShouldDisable(self._platform,
                                                       self._finder_options))

  def testAndroidNotWebviewReturnsFalseOnNotAndroid(self):
    self._platform.SetOSName('not_android')
    self.assertFalse(
        expectations.ANDROID_NOT_WEBVIEW.ShouldDisable(self._platform,
                                                       self._finder_options))
  def testMac1011ReturnsTrueOnMac1011(self):
    self._platform.SetOSName('mac')
    self._platform.SetOsVersionDetailString('10.11')
    self.assertTrue(
        expectations.MAC_10_11.ShouldDisable(self._platform,
                                             self._finder_options))

  def testMac1011ReturnsFalseOnNotMac1011(self):
    self._platform.SetOSName('mac')
    self._platform.SetOsVersionDetailString('10.12')
    self.assertFalse(
        expectations.MAC_10_11.ShouldDisable(self._platform,
                                             self._finder_options))

  def testMac1012ReturnsTrueOnMac1012(self):
    self._platform.SetOSName('mac')
    self._platform.SetOsVersionDetailString('10.12')
    self.assertTrue(
        expectations.MAC_10_12.ShouldDisable(self._platform,
                                             self._finder_options))

  def testMac1012ReturnsFalseOnNotMac1012(self):
    self._platform.SetOSName('mac')
    self._platform.SetOsVersionDetailString('10.11')
    self.assertFalse(
        expectations.MAC_10_12.ShouldDisable(self._platform,
                                             self._finder_options))

  def testNexus5XWebviewFalseOnNotWebview(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android'
    self._platform.SetDeviceTypeName('Nexus 5X')
    self.assertFalse(
        expectations.ANDROID_NEXUS5X_WEBVIEW.ShouldDisable(
            self._platform, self._finder_options))

  def testNexus5XWebviewFalseOnNotNexus5X(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android-webview'
    self.assertFalse(
        expectations.ANDROID_NEXUS5X_WEBVIEW.ShouldDisable(
            self._platform, self._finder_options))

  def testNexus5XWebviewReturnsTrue(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android-webview'
    self._platform.SetDeviceTypeName('Nexus 5X')
    self.assertTrue(
        expectations.ANDROID_NEXUS5X_WEBVIEW.ShouldDisable(
            self._platform, self._finder_options))

  def testNexus6WebviewFalseOnNotWebview(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android'
    self._platform.SetDeviceTypeName('Nexus 6')
    self.assertFalse(
        expectations.ANDROID_NEXUS6_WEBVIEW.ShouldDisable(
            self._platform, self._finder_options))

  def testNexus6WebviewFalseOnNotNexus6(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android-webview'
    self._platform.SetDeviceTypeName('Nexus 5X')
    self.assertFalse(
        expectations.ANDROID_NEXUS6_WEBVIEW.ShouldDisable(
            self._platform, self._finder_options))

  def testNexus6WebviewReturnsTrue(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android-webview'
    self._platform.SetDeviceTypeName('Nexus 6')
    self.assertTrue(
        expectations.ANDROID_NEXUS6_WEBVIEW.ShouldDisable(
            self._platform, self._finder_options))

  def testAndroidNexus6AOSP(self):
    self._platform.SetOSName('android')
    self._platform.SetDeviceTypeName('AOSP on Shamu')
    self.assertTrue(
        expectations.ANDROID_NEXUS6.ShouldDisable(
            self._platform, self._finder_options))

  def testAndroidNexus5XAOSP(self):
    self._platform.SetOSName('android')
    self._platform.SetDeviceTypeName('AOSP on BullHead')
    self.assertTrue(
        expectations.ANDROID_NEXUS5X.ShouldDisable(
            self._platform, self._finder_options))

  def testAndroidNexus6WebviewAOSP(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android-webview'
    self._platform.SetDeviceTypeName('AOSP on Shamu')
    self.assertTrue(
        expectations.ANDROID_NEXUS6_WEBVIEW.ShouldDisable(
            self._platform, self._finder_options))

  def testAndroidNexus5XWebviewAOSP(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android-webview'
    self._platform.SetDeviceTypeName('AOSP on BullHead')
    self.assertTrue(
        expectations.ANDROID_NEXUS5X_WEBVIEW.ShouldDisable(
            self._platform, self._finder_options))

  def testWin7(self):
    self._platform.SetOSName('win')
    self._platform.SetOSVersionName(os_version_module.WIN7)
    self.assertTrue(
        expectations.WIN_7.ShouldDisable(
            self._platform, self._finder_options))
    self.assertEqual('Win 7', str(expectations.WIN_7))

  def testWin10(self):
    self._platform.SetOSName('win')
    self._platform.SetOSVersionName(os_version_module.WIN10)
    self.assertTrue(
        expectations.WIN_10.ShouldDisable(
            self._platform, self._finder_options))
    self.assertEqual('Win 10', str(expectations.WIN_10))

  def testAndroidGoWebviewFalseOnNotWebview(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android'
    self._platform.SetDeviceTypeName('gobo')
    self.assertFalse(
        expectations.ANDROID_GO_WEBVIEW.ShouldDisable(
            self._platform, self._finder_options))

  def testAndroidGoWebviewFalseOnNotNexus6(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android-webview'
    self._platform.SetDeviceTypeName('Nexus 5X')
    self.assertFalse(
        expectations.ANDROID_GO_WEBVIEW.ShouldDisable(
            self._platform, self._finder_options))

  def testAndroidGoWebviewReturnsTrue(self):
    self._platform.SetOSName('android')
    self._finder_options.browser_type = 'android-webview'
    self._platform.SetDeviceTypeName('gobo')
    self.assertTrue(
        expectations.ANDROID_GO_WEBVIEW.ShouldDisable(
            self._platform, self._finder_options))
