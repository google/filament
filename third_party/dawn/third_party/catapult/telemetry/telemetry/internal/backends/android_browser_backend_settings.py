# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import os
import sys
from typing import List, Optional

import dataclasses  # Built-in, but pylint gives an ordering false positive.

from telemetry.core import util

from devil.android import apk_helper
from devil.android.sdk import version_codes

import py_utils


_ANDROID_EARLIEST = 0
_ANDROID_T = 33
_ANDROID_LATEST = sys.maxsize


@dataclasses.dataclass
class _SdkDependentActivity:
  """Specifies when a certain activity should be used based on SDK version.

  |activity_name| will be used if the SDK version is >= |min_version| and
  < |max_version|.
  """
  activity_name: str
  min_version: int
  max_version: int

  def ValidForSdkVersion(self, sdk_version: int) -> bool:
    return self.min_version <= sdk_version < self.max_version


@dataclasses.dataclass
class _SdkDependentAction:
  """Specifies when a certain action type should be used based on SDK version.

  |action_name_ will be used if the SDK version is >= |min_version| and
  < |max_version|.
  """
  action_name: Optional[str]
  min_version: int
  max_version: int

  def ValidForSdkVersion(self, sdk_version: int) -> bool:
    return self.min_version <= sdk_version < self.max_version


@dataclasses.dataclass
class _BackendSettings:
  browser_type: str
  package: str
  command_line_name: str
  devtools_port: str
  apk_name: Optional[str]
  supports_tab_control: bool
  supports_spki_list: bool
  embedder_apk_name: Optional[str] = None
  additional_apk_name: Optional[str] = None
  sdk_dependent_activities: List[_SdkDependentActivity] = dataclasses.field(
      default_factory=list)
  sdk_dependent_actions: List[_SdkDependentAction] = dataclasses.field(
      default_factory=list)

  @property
  def activity(self):
    # Kept for backwards-compatibility until all remaining uses are switched to
    # the SDK-aware version.
    # TODO(crbug.com/332350951): Remove this once all uses are converted.
    assert self.sdk_dependent_activities
    return self.sdk_dependent_activities[0].activity_name


class AndroidBrowserBackendSettings(_BackendSettings):
  """Base class for backend settings of Android browsers.

  These abstract away the differences that may exist between different
  browsers that may be installed or controlled on Android.

  Args:
    browser_type: The short browser name used by Telemetry to identify
      this from, e.g., the --browser command line argument.
    package: The package name used to identify the browser on Android.
      Each browser_type must match to a single package name and viceversa.
    activity: The activity name used to launch this browser via an intent.
    command_line_name: File name where the browser reads command line flags.
    devtools_port: Default remote port used to set up a DevTools conneciton.
      Subclasses may override how this value is interpreted.
    apk_name: Default apk name as built on a chromium checkout, used to
      find local apks on the host platform. Subclasses may override
      how this value is interpreted.
    embedder_apk_name: Name of an additional apk needed, also expected to be
      found in the chromium checkout, used as an app which embbeds e.g.
      the WebView implementation given by the apk_name above.
    supports_tab_control: Whether this browser variant supports the concept
      of tabs.
    supports_spki_list: Whether this browser supports spki-list for ignoring
      certificate errors. See: crbug.com/753948
  """
  __slots__ = ()

  def __init__(self, *args, **kwargs):
    # This can be enforced via dataclasses' kw_only option when on Python 3.10
    # or later.
    assert not args
    super().__init__(*args, **kwargs)

  def __str__(self):
    return '%s (%s)' % (self.browser_type, self.package)

  @property
  def profile_ignore_list(self):
    # Don't delete lib, since it is created by the installer.
    return ('lib', )

  @property
  def requires_embedder(self):
    return self.embedder_apk_name is not None

  @property
  def has_additional_apk(self):
    return self.additional_apk_name is not None

  def GetDevtoolsRemotePort(self, device, package=None):
    del device, package
    # By default return the devtools_port defined in the constructor.
    return self.devtools_port

  def GetApkName(self, device):
    # Subclasses may override this method to pick the correct apk based on
    # specific device features.
    del device  # Unused.
    return self.apk_name

  def FindLocalApk(self, device, chrome_root):
    apk_name = self.GetApkName(device)
    logging.info('Picked apk name %s for browser_type %s',
                 apk_name, self.browser_type)
    if apk_name is None:
      return None
    return util.FindLatestApkOnHost(chrome_root, apk_name)

  # returns True if this is a WebView browser and WebView-specific
  # field trial configurations should apply.
  def IsWebView(self):
    return False

  def GetActivityNameForSdk(self, sdk_version):
    for activity in self.sdk_dependent_activities:
      if activity.ValidForSdkVersion(sdk_version):
        return activity.activity_name
    raise RuntimeError(
        f'No valid activity name found for SDK version {sdk_version}')

  def GetActionForSdk(self, sdk_version):
    for action in self.sdk_dependent_actions:
      if action.ValidForSdkVersion(sdk_version):
        return action.action_name
    raise RuntimeError(f'No valid action found for SDK version {sdk_version}')


class GenericChromeBackendSettings(AndroidBrowserBackendSettings):

  def __init__(self, *args, **kwargs):
    # Provide some defaults common to Chrome-based backends.
    # Android T+ has intent filters, so we need to use a different activity and
    # action.
    kwargs.setdefault('sdk_dependent_activities', [
        _SdkDependentActivity('com.google.android.apps.chrome.Main',
                              _ANDROID_EARLIEST, _ANDROID_T),
        _SdkDependentActivity('com.google.android.apps.chrome.IntentDispatcher',
                              _ANDROID_T, _ANDROID_LATEST),
    ])
    kwargs.setdefault('sdk_dependent_actions', [
        _SdkDependentAction(None, _ANDROID_EARLIEST, _ANDROID_T),
        _SdkDependentAction('android.intent.action.VIEW', _ANDROID_T,
                            _ANDROID_LATEST),
    ])
    kwargs.setdefault('command_line_name', 'chrome-command-line')
    kwargs.setdefault('devtools_port', 'localabstract:chrome_devtools_remote')
    kwargs.setdefault('apk_name', None)
    kwargs.setdefault('embedder_apk_name', None)
    kwargs.setdefault('supports_tab_control', True)
    kwargs.setdefault('supports_spki_list', True)
    kwargs.setdefault('additional_apk_name', None)
    super().__init__(*args, **kwargs)


class GenericChromeBundleBackendSettings(GenericChromeBackendSettings):
  def GetApkName(self, device):
    assert self.apk_name.endswith('_bundle')
    del device  # unused
    # Bundles are created using the generated tool in the output directory's
    # bin directory instead of being output to the apk directory at compile
    # time like a normal APK.
    return os.path.join('..', 'bin', self.apk_name)

  def FindSupportApks(self, apk_path):
    # Trichrome bundles also require their corresponding library apks to be
    # installed, but the library apks are in the apks directory and the bundles
    # are in the bin directory.
    if apk_path is None or self.additional_apk_name is None:
      return []
    additional_apk_path = os.path.normpath(
        os.path.join(os.path.dirname(apk_path), '..', 'apks',
                     self.additional_apk_name))
    assert os.path.exists(additional_apk_path), (
        f'{additional_apk_path} is missing for {apk_path}')
    return [additional_apk_path]


class ChromeBackendSettings(GenericChromeBackendSettings):
  def GetApkName(self, device):
    assert self.apk_name is None
    # The APK to install depends on the OS version of the deivce.
    if device.build_version_sdk >= version_codes.NOUGAT:
      return 'Monochrome.apk'
    return 'Chrome.apk'


class WebViewBasedBackendSettings(AndroidBrowserBackendSettings):

  def __init__(self, *args, **kwargs):
    # Provide some defaults common to WebView based backends.
    kwargs.setdefault('devtools_port',
                      'localabstract:webview_devtools_remote_{pid}')
    kwargs.setdefault('apk_name', None)
    kwargs.setdefault('embedder_apk_name', None)
    kwargs.setdefault('supports_tab_control', False)
    # TODO(crbug.com/753948): Switch to True when spki-list support is
    # implemented on WebView.
    kwargs.setdefault('supports_spki_list', False)
    kwargs.setdefault('additional_apk_name', None)
    super().__init__(*args, **kwargs)

  def GetDevtoolsRemotePort(self, device, package=None):
    # The DevTools port for WebView based backends depends on the browser PID.
    def get_activity_pid():
      return device.GetApplicationPids(package or self.package,
                                       at_most_one=True)

    pid = py_utils.WaitFor(get_activity_pid, timeout=30)
    return self.devtools_port.format(pid=pid)

  def GetEmbedderPackageName(self, finder_options):
    """Get the embedder's package name from it's apk.

    Args:
      finder_options: Telemetry options.
      device: DeviceUtils instance.

    Returns:
      WebView embedder package name."""
    apk_path = util.FindLatestApkOnHost(finder_options.chrome_root,
                                        self.embedder_apk_name)
    if not apk_path:
      # If an apk cannot be found then return
      # the hard coded package name.
      return self.package
    return apk_helper.GetPackageName(apk_path)


class WebViewBackendSettings(WebViewBasedBackendSettings):

  def __init__(self, *args, **kwargs):
    # Provide some defaults for backends that work via system_webview_shell,
    # a testing app with source code available at:
    # https://cs.chromium.org/chromium/src/android_webview/tools/system_webview_shell
    kwargs.setdefault('package', 'org.chromium.webview_shell')
    kwargs.setdefault('sdk_dependent_activities', [
        _SdkDependentActivity('org.chromium.webview_shell.TelemetryActivity',
                              _ANDROID_EARLIEST, _ANDROID_LATEST)
    ])
    kwargs.setdefault(
        'sdk_dependent_actions',
        [_SdkDependentAction(None, _ANDROID_EARLIEST, _ANDROID_LATEST)])
    kwargs.setdefault('embedder_apk_name', 'SystemWebViewShell.apk')
    kwargs.setdefault('command_line_name', 'webview-command-line')
    super().__init__(*args, **kwargs)

  def GetApkName(self, device):
    if self.apk_name is not None:
      return self.apk_name
    # The APK to install depends on the OS version of the deivce unless
    # explicitly overridden.
    if device.build_version_sdk >= version_codes.NOUGAT:
      return 'MonochromePublic.apk'
    return 'SystemWebView.apk'

  def FindSupportApks(self, apk_path):
    all_apks = []
    # Try to find the WebView embedder next to the local APK found.
    if apk_path is not None:
      embedder_apk_path = os.path.join(
          os.path.dirname(apk_path), self.embedder_apk_name)
      if os.path.exists(embedder_apk_path):
        all_apks.append(embedder_apk_path)
      if self.additional_apk_name is not None:
        additional_apk_path = os.path.join(
            os.path.dirname(apk_path), self.additional_apk_name)
        if os.path.exists(additional_apk_path):
          all_apks.append(additional_apk_path)
    return all_apks

  def IsWebView(self):
    return True


class WebViewGoogleBackendSettings(WebViewBackendSettings):
  def GetApkName(self, device):
    assert self.apk_name is None
    # The APK to install depends on the OS version of the deivce.
    if device.build_version_sdk >= version_codes.NOUGAT:
      return 'Monochrome.apk'
    return 'SystemWebViewGoogle.apk'


class WebViewBundleBackendSettings(WebViewBackendSettings):
  def GetApkName(self, device):
    assert self.apk_name.endswith('_bundle')
    del device  # unused
    # Bundles are created using the generated tool in the output directory's
    # bin directory instead of being output to the apk directory at compile
    # time like a normal APK.
    return os.path.join('..', 'bin', self.apk_name)

  def FindSupportApks(self, apk_path):
    # Try to find the WebView embedder in apk directory
    all_apks = []
    if apk_path is not None:
      embedder_apk_path = os.path.join(
          os.path.dirname(apk_path), '..', 'apks', self.embedder_apk_name)
      if os.path.exists(embedder_apk_path):
        all_apks.append(embedder_apk_path)
      if self.additional_apk_name is not None:
        additional_apk_path = os.path.join(
            os.path.dirname(apk_path), '..', 'apks', self.additional_apk_name)
        if os.path.exists(additional_apk_path):
          all_apks.append(additional_apk_path)
    return all_apks


ANDROID_CONTENT_SHELL = AndroidBrowserBackendSettings(
    browser_type='android-content-shell',
    package='org.chromium.content_shell_apk',
    #activity='org.chromium.content_shell_apk.ContentShellActivity',
    sdk_dependent_activities=[
        _SdkDependentActivity(
            'org.chromium.content_shell_apk.ContentShellActivity',
            _ANDROID_EARLIEST, _ANDROID_LATEST)
    ],
    sdk_dependent_actions=[
        _SdkDependentAction(None, _ANDROID_EARLIEST, _ANDROID_LATEST)
    ],
    command_line_name='content-shell-command-line',
    devtools_port='localabstract:content_shell_devtools_remote',
    apk_name='ContentShell.apk',
    embedder_apk_name=None,
    supports_tab_control=False,
    supports_spki_list=True,
    additional_apk_name=None)

ANDROID_WEBVIEW = WebViewBackendSettings(
    browser_type='android-webview')

ANDROID_WEBVIEW_STANDALONE = WebViewBackendSettings(
    apk_name='SystemWebView.apk',
    browser_type='android-webview-standalone')

ANDROID_WEBVIEW_STANDALONE_BUNDLE = WebViewBundleBackendSettings(
    browser_type='android-webview-standalone-bundle',
    apk_name='system_webview_bundle')

ANDROID_WEBVIEW_TRICHROME = WebViewBackendSettings(
    apk_name='TrichromeWebView.apk',
    additional_apk_name='TrichromeLibrary.apk',
    browser_type='android-webview-trichrome')

ANDROID_WEBVIEW_MONOCHROME = WebViewBackendSettings(
    apk_name='MonochromePublic.apk',
    browser_type='android-webview-monochrome')

ANDROID_WEBVIEW_TRICHROME_BUNDLE = WebViewBackendSettings(
    apk_name='trichrome_webview_bundle',
    additional_apk_name='TrichromeLibrary.apk',
    browser_type='android-webview-trichrome-bundle')

ANDROID_WEBVIEW_BUNDLE = WebViewBundleBackendSettings(
    browser_type='android-webview-bundle',
    apk_name='monochrome_public_bundle')

ANDROID_WEBVIEW_GOOGLE = WebViewGoogleBackendSettings(
    browser_type='android-webview-google')

ANDROID_WEBVIEW_GOOGLE_BUNDLE = WebViewBundleBackendSettings(
    browser_type='android-webview-google-bundle',
    apk_name='monochrome_bundle')

ANDROID_WEBVIEW_STANDALONE_GOOGLE = WebViewBackendSettings(
    apk_name='SystemWebViewGoogle.apk',
    browser_type='android-webview-standalone-google')

ANDROID_WEBVIEW_STANDALONE_GOOGLE_BUNDLE = WebViewBundleBackendSettings(
    browser_type='android-webview-standalone-google-bundle',
    apk_name='system_webview_google_bundle')

ANDROID_WEBVIEW_TRICHROME_GOOGLE = WebViewBackendSettings(
    apk_name='TrichromeWebViewGoogle.apk',
    additional_apk_name='TrichromeLibraryGoogle.apk',
    browser_type='android-webview-trichrome-google')

ANDROID_WEBVIEW_TRICHROME_GOOGLE_BUNDLE = WebViewBundleBackendSettings(
    apk_name='trichrome_webview_google_bundle',
    additional_apk_name='TrichromeLibraryGoogle6432.apk',
    browser_type='android-webview-trichrome-google-bundle')

ANDROID_WEBVIEW_INSTRUMENTATION = WebViewBasedBackendSettings(
    browser_type='android-webview-instrumentation',
    package='org.chromium.android_webview.shell',
    sdk_dependent_activities=[
        _SdkDependentActivity(
            'org.chromium.android_webview.shell.AwShellActivity',
            _ANDROID_EARLIEST, _ANDROID_LATEST)
    ],
    sdk_dependent_actions=[
        _SdkDependentAction(None, _ANDROID_EARLIEST, _ANDROID_LATEST)
    ],
    #activity='org.chromium.android_webview.shell.AwShellActivity',
    command_line_name='android-webview-command-line',
    apk_name='WebViewInstrumentation.apk')

ANDROID_CHROMIUM = GenericChromeBackendSettings(
    browser_type='android-chromium',
    package='org.chromium.chrome',
    apk_name='ChromePublic.apk')

ANDROID_CHROMIUM_BUNDLE = GenericChromeBundleBackendSettings(
    browser_type='android-chromium-bundle',
    package='org.chromium.chrome',
    apk_name='monochrome_public_bundle')

ANDROID_CHROMIUM_MONOCHROME = GenericChromeBackendSettings(
    browser_type='android-chromium-monochrome',
    package='org.chromium.chrome',
    apk_name='MonochromePublic.apk'
)

ANDROID_CHROMIUM_BETA = GenericChromeBackendSettings(
    browser_type='android-chromium.beta',
    package='org.chromium.chrome.beta',
    apk_name='ChromePublic.apk')

ANDROID_CHROMIUM_BUNDLE_BETA = GenericChromeBundleBackendSettings(
    browser_type='android-chromium-bundle.beta',
    package='org.chromium.chrome.beta',
    apk_name='monochrome_public_bundle')

ANDROID_CHROMIUM_MONOCHROME_BETA = GenericChromeBackendSettings(
    browser_type='android-chromium-monochrome.beta',
    package='org.chromium.chrome.beta',
    apk_name='MonochromePublic.apk'
)

ANDROID_CHROMIUM_CANARY = GenericChromeBackendSettings(
    browser_type='android-chromium.canary',
    package='org.chromium.chrome.canary',
    apk_name='ChromePublic.apk')

ANDROID_CHROMIUM_BUNDLE_CANARY = GenericChromeBundleBackendSettings(
    browser_type='android-chromium-bundle.canary',
    package='org.chromium.chrome.canary',
    apk_name='monochrome_public_bundle')

ANDROID_CHROMIUM_MONOCHROME_CANARY = GenericChromeBackendSettings(
    browser_type='android-chromium-monochrome.canary',
    package='org.chromium.chrome.canary',
    apk_name='MonochromePublic.apk'
)

ANDROID_CHROMIUM_DEV = GenericChromeBackendSettings(
    browser_type='android-chromium.dev',
    package='org.chromium.chrome.dev',
    apk_name='ChromePublic.apk')

ANDROID_CHROMIUM_BUNDLE_DEV = GenericChromeBundleBackendSettings(
    browser_type='android-chromium-bundle.dev',
    package='org.chromium.chrome.dev',
    apk_name='monochrome_public_bundle')

ANDROID_CHROMIUM_MONOCHROME_DEV = GenericChromeBackendSettings(
    browser_type='android-chromium-monochrome.dev',
    package='org.chromium.chrome.dev',
    apk_name='MonochromePublic.apk'
)

ANDROID_CHROME = ChromeBackendSettings(
    browser_type='android-chrome',
    package='com.google.android.apps.chrome')

ANDROID_CHROME_BUNDLE = GenericChromeBundleBackendSettings(
    browser_type='android-chrome-bundle',
    package='com.google.android.apps.chrome',
    apk_name='monochrome_bundle')

ANDROID_TRICHROME_BUNDLE = GenericChromeBundleBackendSettings(
    browser_type='android-trichrome-bundle',
    package='com.google.android.apps.chrome',
    apk_name='trichrome_chrome_google_bundle',
    additional_apk_name='TrichromeLibraryGoogle.apk')

ANDROID_TRICHROME_CHROME_GOOGLE_64_32_BUNDLE = (
    GenericChromeBundleBackendSettings(
        browser_type='android-trichrome-chrome-google-64-32-bundle',
        package='com.google.android.apps.chrome',
        apk_name='trichrome_chrome_google_64_32_bundle',
        additional_apk_name='TrichromeLibraryGoogle6432.apk'))

ANDROID_CHROME_64_BUNDLE = GenericChromeBundleBackendSettings(
    browser_type='android-chrome-64-bundle',
    package='com.google.android.apps.chrome',
    apk_name='monochrome_64_32_bundle')

ANDROID_CHROME_BETA = GenericChromeBackendSettings(
    browser_type='android-chrome-beta',
    package='com.chrome.beta')

ANDROID_CHROME_DEV = GenericChromeBackendSettings(
    browser_type='android-chrome-dev',
    package='com.chrome.dev')

ANDROID_CHROME_CANARY = GenericChromeBackendSettings(
    browser_type='android-chrome-canary',
    package='com.chrome.canary')

ANDROID_SYSTEM_CHROME = GenericChromeBackendSettings(
    browser_type='android-system-chrome',
    package='com.android.chrome')


ANDROID_BACKEND_SETTINGS = (
    ANDROID_CONTENT_SHELL,
    ANDROID_WEBVIEW,
    ANDROID_WEBVIEW_BUNDLE,
    ANDROID_WEBVIEW_GOOGLE,
    ANDROID_WEBVIEW_GOOGLE_BUNDLE,
    ANDROID_WEBVIEW_INSTRUMENTATION,
    ANDROID_WEBVIEW_MONOCHROME,
    ANDROID_WEBVIEW_STANDALONE,
    ANDROID_WEBVIEW_STANDALONE_BUNDLE,
    ANDROID_WEBVIEW_STANDALONE_GOOGLE,
    ANDROID_WEBVIEW_STANDALONE_GOOGLE_BUNDLE,
    ANDROID_WEBVIEW_TRICHROME,
    ANDROID_WEBVIEW_TRICHROME_BUNDLE,
    ANDROID_WEBVIEW_TRICHROME_GOOGLE,
    ANDROID_WEBVIEW_TRICHROME_GOOGLE_BUNDLE,
    ANDROID_CHROMIUM,
    ANDROID_CHROMIUM_BUNDLE,
    ANDROID_CHROMIUM_MONOCHROME,
    ANDROID_CHROMIUM_BETA,
    ANDROID_CHROMIUM_BUNDLE_BETA,
    ANDROID_CHROMIUM_MONOCHROME_BETA,
    ANDROID_CHROMIUM_CANARY,
    ANDROID_CHROMIUM_BUNDLE_CANARY,
    ANDROID_CHROMIUM_MONOCHROME_CANARY,
    ANDROID_CHROMIUM_DEV,
    ANDROID_CHROMIUM_BUNDLE_DEV,
    ANDROID_CHROMIUM_MONOCHROME_DEV,
    ANDROID_CHROME,
    ANDROID_CHROME_64_BUNDLE,
    ANDROID_CHROME_BUNDLE,
    ANDROID_TRICHROME_BUNDLE,
    ANDROID_TRICHROME_CHROME_GOOGLE_64_32_BUNDLE,
    ANDROID_CHROME_BETA,
    ANDROID_CHROME_DEV,
    ANDROID_CHROME_CANARY,
    ANDROID_SYSTEM_CHROME
)
