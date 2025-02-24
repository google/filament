# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Finds browsers that can be controlled by telemetry."""

from __future__ import absolute_import
import logging
import time

from telemetry import decorators
from telemetry.internal.backends.chrome import android_browser_finder
from telemetry.internal.backends.chrome import cros_browser_finder
from telemetry.internal.backends.chrome import desktop_browser_finder
from telemetry.internal.backends.chrome import fuchsia_browser_finder
from telemetry.internal.browser import browser_finder_exceptions
from telemetry.internal.platform import device_finder

BROWSER_FINDERS = [
    desktop_browser_finder,
    android_browser_finder,
    cros_browser_finder,
    fuchsia_browser_finder,
]


def _GetBrowserFinders(supported_platforms):
  if not supported_platforms or 'all' in supported_platforms:
    return BROWSER_FINDERS
  browser_finders = []
  if any(p in supported_platforms for p in ['mac', 'linux', 'win']):
    browser_finders.append(desktop_browser_finder)
  if 'android' in supported_platforms:
    browser_finders.append(android_browser_finder)
  if 'chromeos' in supported_platforms:
    browser_finders.append(cros_browser_finder)
  if 'fuchsia' in supported_platforms:
    browser_finders.append(fuchsia_browser_finder)
  return browser_finders


def FindAllBrowserTypes(browser_finders=None):
  browsers = []
  if not browser_finders:
    browser_finders = BROWSER_FINDERS
  for bf in browser_finders:
    browsers.extend(bf.FindAllBrowserTypes())
  return browsers

def _IsCrosBrowser(options):
  return (options.browser_type in
          ['cros-chrome', 'cros-chrome-guest', 'lacros-chrome'])

def SetTargetPlatformsBasedOnBrowserType(options):
  """Sets options.target_platforms based on options.browser_type

  This is in an effort to avoid doing unnecessary work, e.g. looking for Android
  devices if a desktop browser is specified.

  Args:
    options: A BrowserFinderOptions instance.
  """
  browser_type = options.browser_type
  if options.target_platforms or not browser_type or browser_type == 'list':
    return

  options.target_platforms = []
  if browser_type in desktop_browser_finder.FindAllBrowserTypes():
    options.target_platforms.extend(['linux', 'mac', 'win'])
  if browser_type in android_browser_finder.FindAllBrowserTypes():
    options.target_platforms.append('android')
  if browser_type in cros_browser_finder.FindAllBrowserTypes():
    options.target_platforms.append('chromeos')
  if browser_type in fuchsia_browser_finder.FindAllBrowserTypes():
    options.target_platforms.append('fuchsia')

@decorators.Cache
def FindBrowser(options):
  """Finds the best PossibleBrowser object given a BrowserOptions object.

  Args:
    A BrowserFinderOptions object.

  Returns:
    A PossibleBrowser object. None if browser does not exist in DUT.

  Raises:
    BrowserFinderException: Options improperly set, or an error occurred.
  """
  if options.__class__.__name__ == '_FakeBrowserFinderOptions':
    return options.fake_possible_browser
  if options.browser_type == 'exact' and options.browser_executable is None:
    raise browser_finder_exceptions.BrowserFinderException(
        '--browser=exact requires --browser-executable to be set.')
  if options.browser_type != 'exact' and options.browser_executable is not None:
    raise browser_finder_exceptions.BrowserFinderException(
        '--browser-executable requires --browser=exact.')

  if (not _IsCrosBrowser(options)
      and (options.remote is not None or options.fetch_cros_remote)):
    raise browser_finder_exceptions.BrowserFinderException(
        '--remote requires --browser=[la]cros-chrome[-guest].')

  SetTargetPlatformsBasedOnBrowserType(options)
  devices = []
  for iteration in range(options.initial_find_device_attempts):
    devices = device_finder.GetDevicesMatchingOptions(options)
    if devices:
      break
    if iteration + 1 < options.initial_find_device_attempts:
      logging.warning('Did not find any devices while looking for browsers, '
                      'retrying after waiting a bit.')
      time.sleep(10)
  browsers = []
  default_browsers = []

  browser_finders = _GetBrowserFinders(options.target_platforms)

  for device in devices:
    for finder in browser_finders:
      if(options.browser_type and options.browser_type != 'any' and
         options.browser_type not in finder.FindAllBrowserTypes()):
        continue
      curr_browsers = finder.FindAllAvailableBrowsers(options, device)
      new_default_browser = finder.SelectDefaultBrowser(curr_browsers)
      if new_default_browser:
        default_browsers.append(new_default_browser)
      browsers.extend(curr_browsers)

  if not browsers:
    return None

  if options.browser_type is None:
    if default_browsers:
      default_browser = max(default_browsers,
                            key=lambda b: b.last_modification_time)
      logging.warning('--browser omitted. Using most recent local build: %s',
                      default_browser.browser_type)
      default_browser.UpdateExecutableIfNeeded()
      return default_browser

    if len(browsers) == 1:
      logging.warning('--browser omitted. Using only available browser: %s',
                      browsers[0].browser_type)
      browsers[0].UpdateExecutableIfNeeded()
      return browsers[0]

    raise browser_finder_exceptions.BrowserTypeRequiredException(
        '--browser must be specified. Available browsers:\n%s' %
        '\n'.join(sorted({b.browser_type for b in browsers})))

  chosen_browser = None
  if options.browser_type == 'any':
    types = FindAllBrowserTypes(browser_finders)
    chosen_browser = min(browsers, key=lambda b: types.index(b.browser_type))
  else:
    matching_browsers = [
        b for b in browsers
        if b.browser_type == options.browser_type and
        b.SupportsOptions(options.browser_options)]
    if not matching_browsers:
      logging.warning('Cannot find any matched browser')
      return None
    if len(matching_browsers) > 1:
      logging.warning('Multiple browsers of the same type found: %r',
                      matching_browsers)
    chosen_browser = max(matching_browsers,
                         key=lambda b: b.last_modification_time)

  if chosen_browser:
    logging.info('Chose browser: %r', chosen_browser)
    chosen_browser.UpdateExecutableIfNeeded()

  return chosen_browser


@decorators.Cache
def GetAllAvailableBrowsers(options, device):
  """Returns a list of available browsers on the device.

  Args:
    options: A BrowserOptions object.
    device: The target device, which can be None.

  Returns:
    A list of browser instances.

  Raises:
    BrowserFinderException: Options are improperly set, or an error occurred.
  """
  if not device:
    return []
  possible_browsers = []
  for browser_finder in BROWSER_FINDERS:
    possible_browsers.extend(
        browser_finder.FindAllAvailableBrowsers(options, device))
  return possible_browsers


@decorators.Cache
def GetAllAvailableBrowserTypes(options):
  """Returns a list of available browser types.

  Args:
    options: A BrowserOptions object.

  Returns:
    A list of browser type strings.

  Raises:
    BrowserFinderException: Options are improperly set, or an error occurred.
  """
  devices = device_finder.GetDevicesMatchingOptions(options)
  possible_browsers = []
  for device in devices:
    possible_browsers.extend(GetAllAvailableBrowsers(options, device))
  type_list = {browser.browser_type for browser in possible_browsers}
  # The reference build should be available for mac, linux and win, but the
  # desktop browser finder won't return it in the list of browsers.
  for browser in possible_browsers:
    if (browser.target_os == 'darwin' or browser.target_os.startswith('linux')
        or browser.target_os.startswith('win')):
      type_list.add('reference')
      break
  return sorted(list(type_list))
