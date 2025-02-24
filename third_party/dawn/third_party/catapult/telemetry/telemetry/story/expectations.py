# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.core import os_version as os_version_module


# TODO(rnephew): Since TestConditions are being used for more than
# just story expectations now, this should be decoupled and refactored
# to be clearer.
class _TestCondition():
  def ShouldDisable(self, platform, finder_options):
    raise NotImplementedError

  def __str__(self):
    raise NotImplementedError

  def GetSupportedPlatformNames(self):
    """Returns a set of supported platforms' names."""
    raise NotImplementedError


class _TestConditionByPlatformList(_TestCondition):
  def __init__(self, platforms, name):
    self._platforms = platforms
    self._name = name

  def ShouldDisable(self, platform, finder_options):
    del finder_options  # Unused.
    return platform.GetOSName() in self._platforms

  def __str__(self):
    return self._name

  def GetSupportedPlatformNames(self):
    return set(self._platforms)


class _AllTestCondition(_TestCondition):
  def ShouldDisable(self, platform, finder_options):
    del platform, finder_options  # Unused.
    return True

  def __str__(self):
    return 'All'

  def GetSupportedPlatformNames(self):
    return {'all'}


class _TestConditionAndroidSvelte(_TestCondition):
  """Matches android devices with a svelte (low-memory) build."""
  def ShouldDisable(self, platform, finder_options):
    del finder_options  # Unused.
    return platform.GetOSName() == 'android' and platform.IsSvelte()

  def __str__(self):
    return 'Android Svelte'

  def GetSupportedPlatformNames(self):
    return {'android'}

class _TestConditionByAndroidModel(_TestCondition):
  def __init__(self, model, name=None):
    self._model = model
    self._name = name if name else model

  def ShouldDisable(self, platform, finder_options):
    return (platform.GetOSName() == 'android' and
            self._model == platform.GetDeviceTypeName())

  def __str__(self):
    return self._name

  def GetSupportedPlatformNames(self):
    return {'android'}

class _TestConditionAndroidWebview(_TestCondition):
  def ShouldDisable(self, platform, finder_options):
    return (platform.GetOSName() == 'android' and
            finder_options.browser_type.startswith('android-webview'))

  def __str__(self):
    return 'Android Webview'

  def GetSupportedPlatformNames(self):
    return {'android'}

class _TestConditionAndroidNotWebview(_TestCondition):
  def ShouldDisable(self, platform, finder_options):
    return (platform.GetOSName() == 'android' and not
            finder_options.browser_type.startswith('android-webview'))

  def __str__(self):
    return 'Android but not webview'

  def GetSupportedPlatformNames(self):
    return {'android'}

class _TestConditionByMacVersion(_TestCondition):
  def __init__(self, version, name=None):
    self._version = version
    self._name = name

  def __str__(self):
    return self._name

  def GetSupportedPlatformNames(self):
    return {'mac'}

  def ShouldDisable(self, platform, finder_options):
    if platform.GetOSName() != 'mac':
      return False
    return platform.GetOSVersionDetailString().startswith(self._version)


class _TestConditionByWinVersion(_TestCondition):
  def __init__(self, version, name):
    self._version = version
    self._name = name

  def __str__(self):
    return self._name

  def GetSupportedPlatformNames(self):
    return {'win'}

  def ShouldDisable(self, platform, finder_options):
    if platform.GetOSName() != 'win':
      return False
    return platform.GetOSVersionName() == self._version


class _TestConditionFuchsiaWebEngineShell(_TestCondition):
  def ShouldDisable(self, platform, finder_options):
    return (platform.GetOSName() == 'fuchsia' and
            finder_options.browser_type.startswith('web-engine-shell'))

  def __str__(self):
    return 'Fuchsia with web-engine-shell'

  def GetSupportedPlatformNames(self):
    return {'fuchsia', 'fuchsia-board-astro', 'fuchsia-board-sherlock'}

class _TestConditionFuchsiaCastStreamingShell(_TestCondition):
  def ShouldDisable(self, platform, finder_options):
    return (platform.GetOSName() == 'fuchsia' and
            finder_options.browser_type.startswith('cast-streaming-shell'))

  def __str__(self):
    return 'Fuchsia with cast-streaming-shell'

  def GetSupportedPlatformNames(self):
    return {'fuchsia', 'fuchsia-board-astro', 'fuchsia-board-sherlock'}

class _TestConditionFuchsiaByBoard(_TestCondition):
  def __init__(self, board):
    self._board = 'fuchsia-board-' + board

  def ShouldDisable(self, platform, finder_options):
    return (platform.GetOSName() == 'fuchsia' and
            platform.GetDeviceTypeName() == self._board)

  def __str__(self):
    return 'Fuchsia on ' + self._board

  def GetSupportedPlatformNames(self):
    return {'fuchsia', 'fuchsia-board-' + self._board}


class _TestConditionLogicalAndConditions(_TestCondition):
  def __init__(self, conditions, name):
    self._conditions = conditions
    self._name = name

  def __str__(self):
    return self._name

  def GetSupportedPlatformNames(self):
    platforms = set()
    for cond in self._conditions:
      platforms.update(cond.GetSupportedPlatformNames())
    return platforms

  def ShouldDisable(self, platform, finder_options):
    return all(
        c.ShouldDisable(platform, finder_options) for c in self._conditions)


class _TestConditionLogicalOrConditions(_TestCondition):
  def __init__(self, conditions, name):
    self._conditions = conditions
    self._name = name

  def __str__(self):
    return self._name

  def GetSupportedPlatformNames(self):
    platforms = set()
    for cond in self._conditions:
      platforms.update(cond.GetSupportedPlatformNames())
    return platforms

  def ShouldDisable(self, platform, finder_options):
    return any(
        c.ShouldDisable(platform, finder_options) for c in self._conditions)


ALL = _AllTestCondition()
ALL_MAC = _TestConditionByPlatformList(['mac'], 'Mac')
ALL_WIN = _TestConditionByPlatformList(['win'], 'Win')
WIN_7 = _TestConditionByWinVersion(os_version_module.WIN7, 'Win 7')
WIN_10 = _TestConditionByWinVersion(os_version_module.WIN10, 'Win 10')
WIN_11 = _TestConditionByWinVersion(os_version_module.WIN11, 'Win 11')
ALL_LINUX = _TestConditionByPlatformList(['linux'], 'Linux')
ALL_CHROMEOS = _TestConditionByPlatformList(['chromeos'], 'ChromeOS')
ALL_ANDROID = _TestConditionByPlatformList(['android'], 'Android')
# Fuchsia setup, while similar to mobile, renders, Desktop pages.
ALL_DESKTOP = _TestConditionByPlatformList(
    ['mac', 'linux', 'win', 'chromeos', 'fuchsia'], 'Desktop')
ALL_MOBILE = _TestConditionByPlatformList(['android'], 'Mobile')
ANDROID_NEXUS5 = _TestConditionByAndroidModel('Nexus 5')
_ANDROID_NEXUS5X = _TestConditionByAndroidModel('Nexus 5X')
_ANDROID_NEXUS5XAOSP = _TestConditionByAndroidModel('AOSP on BullHead')
ANDROID_NEXUS5X = _TestConditionLogicalOrConditions(
    [_ANDROID_NEXUS5X, _ANDROID_NEXUS5XAOSP], 'Nexus 5X')
_ANDROID_NEXUS6 = _TestConditionByAndroidModel('Nexus 6')
_ANDROID_NEXUS6AOSP = _TestConditionByAndroidModel('AOSP on Shamu')
ANDROID_NEXUS6 = _TestConditionLogicalOrConditions(
    [_ANDROID_NEXUS6, _ANDROID_NEXUS6AOSP], 'Nexus 6')
ANDROID_NEXUS6P = _TestConditionByAndroidModel('Nexus 6P')
ANDROID_NEXUS7 = _TestConditionByAndroidModel('Nexus 7')
ANDROID_GO = _TestConditionByAndroidModel('gobo', 'Android Go')
ANDROID_ONE = _TestConditionByAndroidModel('W6210', 'Android One')
ANDROID_SVELTE = _TestConditionAndroidSvelte()
ANDROID_LOW_END = _TestConditionLogicalOrConditions(
    [ANDROID_GO, ANDROID_SVELTE, ANDROID_ONE], 'Android Low End')
ANDROID_PIXEL2 = _TestConditionByAndroidModel('Pixel 2')
ANDROID_WEBVIEW = _TestConditionAndroidWebview()
ANDROID_NOT_WEBVIEW = _TestConditionAndroidNotWebview()
# MAC_10_11 Includes:
#   Mac 10.11 Perf, Mac Retina Perf, Mac Pro 10.11 Perf, Mac Air 10.11 Perf
MAC_10_11 = _TestConditionByMacVersion('10.11', 'Mac 10.11')
# Mac 10_12 Includes:
#   Mac 10.12 Perf, Mac Mini 8GB 10.12 Perf
MAC_10_12 = _TestConditionByMacVersion('10.12', 'Mac 10.12')
ANDROID_NEXUS6_WEBVIEW = _TestConditionLogicalAndConditions(
    [ANDROID_NEXUS6, ANDROID_WEBVIEW], 'Nexus6 Webview')
ANDROID_NEXUS5X_WEBVIEW = _TestConditionLogicalAndConditions(
    [ANDROID_NEXUS5X, ANDROID_WEBVIEW], 'Nexus5X Webview')
ANDROID_GO_WEBVIEW = _TestConditionLogicalAndConditions(
    [ANDROID_GO, ANDROID_WEBVIEW], 'Android Go Webview')
ANDROID_PIXEL2_WEBVIEW = _TestConditionLogicalAndConditions(
    [ANDROID_PIXEL2, ANDROID_WEBVIEW], 'Pixel2 Webview')
FUCHSIA_WEB_ENGINE_SHELL = _TestConditionFuchsiaWebEngineShell()
FUCHSIA_CAST_STREAMING_SHELL = _TestConditionFuchsiaCastStreamingShell()
FUCHSIA_ASTRO = _TestConditionFuchsiaByBoard('astro')
FUCHSIA_SHERLOCK = _TestConditionFuchsiaByBoard('sherlock')

EXPECTATION_NAME_MAP = {
    'All': ALL,
    'Android_Go': ANDROID_GO,
    'Android_One': ANDROID_ONE,
    'Android_Svelte': ANDROID_SVELTE,
    'Android_Low_End': ANDROID_LOW_END,
    'Android_Webview': ANDROID_WEBVIEW,
    'Android_but_not_webview': ANDROID_NOT_WEBVIEW,
    'Mac': ALL_MAC,
    'Win': ALL_WIN,
    'Win_7': WIN_7,
    'Win_10': WIN_10,
    'Win_11': WIN_11,
    'Linux': ALL_LINUX,
    'ChromeOS': ALL_CHROMEOS,
    'Android': ALL_ANDROID,
    'Desktop': ALL_DESKTOP,
    'Mobile': ALL_MOBILE,
    'Nexus_5': ANDROID_NEXUS5,
    'Nexus_5X': ANDROID_NEXUS5X,
    'Nexus_6': ANDROID_NEXUS6,
    'Nexus_6P': ANDROID_NEXUS6P,
    'Nexus_7': ANDROID_NEXUS7,
    'Pixel_2': ANDROID_PIXEL2,
    'Mac_10.11': MAC_10_11,
    'Mac_10.12': MAC_10_12,
    'Nexus6_Webview': ANDROID_NEXUS6_WEBVIEW,
    'Nexus5X_Webview': ANDROID_NEXUS5X_WEBVIEW,
    'Android_Go_Webview': ANDROID_GO_WEBVIEW,
    'Pixel2_Webview': ANDROID_PIXEL2_WEBVIEW,
    'Fuchsia_WebEngineShell': FUCHSIA_WEB_ENGINE_SHELL,
    'Fuchsia_CastStreamingShell': FUCHSIA_CAST_STREAMING_SHELL,
    'Fuchsia_Astro': FUCHSIA_ASTRO,
    'Fuchsia_Sherlock': FUCHSIA_SHERLOCK,
}
