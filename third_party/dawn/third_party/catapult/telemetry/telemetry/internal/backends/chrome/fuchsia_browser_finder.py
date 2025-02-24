# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Finds Fuchsia browsers that can be started and controlled by telemetry."""

from __future__ import absolute_import
import os
import platform

from telemetry.core import fuchsia_interface
from telemetry.core import platform as telemetry_platform
from telemetry.internal.backends.chrome import fuchsia_browser_backend
from telemetry.internal.browser import browser
from telemetry.internal.browser import possible_browser
from telemetry.internal.platform import fuchsia_device
from telemetry.internal.backends.chrome import chrome_startup_args
from telemetry.internal.util import local_first_binary_manager


class UnsupportedExtensionException(Exception):
  pass

class PossibleFuchsiaBrowser(possible_browser.PossibleBrowser):

  def __init__(self, browser_type, finder_options, fuchsia_platform):
    del finder_options
    super().__init__(browser_type, 'fuchsia', True)
    self._platform = fuchsia_platform
    self._platform_backend = (
        fuchsia_platform._platform_backend) # pylint: disable=protected-access

    # Like CrOS, there's no way to automatically determine the build directory,
    # so use the manually set output directory if possible.
    self._build_dir = os.environ.get('CHROMIUM_OUTPUT_DIR')

  def __repr__(self):
    return 'PossibleFuchsiaBrowser(app_type=%s)' % self.browser_type

  @property
  def browser_directory(self):
    return None

  @property
  def profile_directory(self):
    return None

  def _InitPlatformIfNeeded(self):
    pass

  def _GetPathsForOsPageCacheFlushing(self):
    # There is no page write-back on Fuchsia, so there is nothing to flush.
    return []

  def Create(self):
    """Start the browser process."""
    if local_first_binary_manager.LocalFirstBinaryManager.NeedsInit():
      local_first_binary_manager.LocalFirstBinaryManager.Init(
          self._build_dir, None, 'linux', platform.machine())

    browser_backend = fuchsia_browser_backend.FuchsiaBrowserBackend(
        self._platform_backend, self._browser_options,
        self.browser_directory, self.profile_directory)
    startup_args = self.GetBrowserStartupArgs(self._browser_options,
                                              browser_backend.browser_type)
    try:
      return browser.Browser(
          browser_backend, self._platform_backend, startup_args,
          find_existing=False)
    except Exception:
      browser_backend.Close()
      raise

  def GetBrowserStartupArgs(self, browser_options, browser_type):
    startup_args = chrome_startup_args.GetFromBrowserOptions(browser_options)

    startup_args.extend(chrome_startup_args.GetReplayArgs(
        self._platform_backend.network_controller_backend))
    # This substitution may no longer be necessary after the change to FFX, but
    # is kept in place until it's shown to cause problems.
    if browser_type == fuchsia_browser_backend.FUCHSIA_CHROME:
      startup_args = [arg.replace('=<-loopback>', '="<-loopback>"')
                      for arg in startup_args]

    return startup_args

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
          'Fuchsia browsers do not support extensions.')
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
    if b.browser_type == fuchsia_browser_backend.WEB_ENGINE_SHELL:
      return b
  return None


def FindAllBrowserTypes():
  return fuchsia_interface.FUCHSIA_BROWSERS


def FindAllAvailableBrowsers(finder_options, device):
  """Finds all available Fuchsia browsers."""
  browsers = []
  if not isinstance(device, fuchsia_device.FuchsiaDevice):
    return browsers

  fuchsia_platform = telemetry_platform.GetPlatformForDevice(device,
                                                             finder_options)
  browsers.extend([
      PossibleFuchsiaBrowser(
          finder_options.browser_type, finder_options, fuchsia_platform)
  ])
  return browsers
