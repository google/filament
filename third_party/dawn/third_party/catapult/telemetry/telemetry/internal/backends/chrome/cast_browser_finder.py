# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Finds browsers that can be Cast to and controlled by telemetry."""

from __future__ import absolute_import
import platform
import sys

from telemetry.core import cast_interface
from telemetry.core import platform as telemetry_platform
from telemetry.internal.browser import browser
from telemetry.internal.browser import possible_browser
from telemetry.internal.backends.chrome import local_cast_browser_backend
from telemetry.internal.backends.chrome import remote_cast_browser_backend
from telemetry.internal.backends.chrome import chrome_startup_args
from telemetry.internal.platform import cast_device
from telemetry.internal.util import local_first_binary_manager


class UnsupportedExtensionException(Exception):
  pass

class PossibleCastBrowser(possible_browser.PossibleBrowser):

  def __init__(self, browser_type, finder_options, cast_platform):
    del finder_options
    super().__init__(browser_type,
                                              sys.platform.lower(), True)
    self._casting_tab = None
    self._platform = cast_platform
    self._platform_backend = (
        cast_platform._platform_backend) # pylint: disable=protected-access

  def __repr__(self):
    return 'PossibleCastBrowser(app_type=%s)' % self.browser_type

  @property
  def browser_directory(self):
    return None

  @property
  def profile_directory(self):
    return None

  def _InitPlatformIfNeeded(self):
    pass

  def _GetPathsForOsPageCacheFlushing(self):
    # Cast browsers don't have a need to flush.
    return []

  def SetCastSender(self, casting_tab):
    self._casting_tab = casting_tab

  def Create(self):
    """Start the browser process."""
    if local_first_binary_manager.LocalFirstBinaryManager.NeedsInit():
      local_first_binary_manager.LocalFirstBinaryManager.Init(
          '', None, 'linux', platform.machine())

    startup_args = chrome_startup_args.GetFromBrowserOptions(
        self._browser_options)
    if self._platform_backend.ip_addr:
      browser_backend = remote_cast_browser_backend.RemoteCastBrowserBackend(
          self._platform_backend, self._browser_options,
          self.browser_directory, self.profile_directory,
          self._casting_tab)
    else:
      browser_backend = local_cast_browser_backend.LocalCastBrowserBackend(
          self._platform_backend, self._browser_options,
          self.browser_directory, self.profile_directory,
          self._casting_tab)
    try:
      return browser.Browser(
          browser_backend, self._platform_backend, startup_args,
          find_existing=False)
    except Exception:
      browser_backend.Close()
      raise

  def CleanUpEnvironment(self):
    if self._browser_options is None:
      return  # No environment to clean up.
    try:
      self._TearDownEnvironment()
    finally:
      self._browser_options = None

  def SupportsOptions(self, browser_options):
    if len(browser_options.extensions_to_load) > 0:
      raise UnsupportedExtensionException(
          'Cast browsers do not support extensions.')
    return True

  def UpdateExecutableIfNeeded(self):
    # Updating the browser is currently handled in the Chromium repository
    # instead of Catapult.
    pass

  @property
  def last_modification_time(self):
    return -1


def SelectDefaultBrowser(possible_browsers):
  for b in possible_browsers:
    if b.browser_type == 'platform_app':
      return b
  return None


def FindAllBrowserTypes():
  return cast_interface.CAST_BROWSERS


def FindAllAvailableBrowsers(finder_options, device):
  """Finds all available Cast browsers."""
  browsers = []
  if not isinstance(device, cast_device.CastDevice):
    return browsers

  cast_platform = telemetry_platform.GetPlatformForDevice(device,
                                                          finder_options)
  browsers.extend([
      PossibleCastBrowser(
          finder_options.cast_receiver_type, finder_options, cast_platform)
  ])
  return browsers
