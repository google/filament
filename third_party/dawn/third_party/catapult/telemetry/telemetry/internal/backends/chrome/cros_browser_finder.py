# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Finds CrOS browsers that can be started and controlled by telemetry."""

from __future__ import absolute_import
import json
import logging
import os
import platform
import posixpath
import random

from telemetry.core import cros_interface
from telemetry.core import linux_based_interface
from telemetry.core import platform as platform_module
from telemetry.internal.backends.chrome import chrome_startup_args
from telemetry.internal.backends.chrome import cros_browser_backend
from telemetry.internal.backends.chrome import cros_browser_with_oobe
from telemetry.internal.backends.chrome import lacros_browser_backend
from telemetry.internal.browser import browser
from telemetry.internal.browser import browser_finder_exceptions
from telemetry.internal.browser import possible_browser
from telemetry.internal.platform import cros_device
from telemetry.internal.util import local_first_binary_manager

from devil.utils import cmd_helper

import py_utils


class PossibleCrOSBrowser(possible_browser.PossibleBrowser):
  """A launchable CrOS browser instance."""

  # The path contains spaces, so we need to quote it. We don't join it with
  # anything in this file, so we can quote it here instead of everywhere it's
  # used.
  _CROS_MINIDUMP_DIR = cmd_helper.SingleQuote(
      cros_interface.CrOSInterface.MINIDUMP_DIR)

  _DEFAULT_CHROME_ENV = [
      'CHROME_HEADLESS=1',
      'BREAKPAD_DUMP_LOCATION=%s' % _CROS_MINIDUMP_DIR,
  ]

  def __init__(self, browser_type, finder_options, cros_platform, is_guest):
    super().__init__(
        browser_type,
        'lacros' if browser_type == 'lacros-chrome' else 'cros', True)
    assert browser_type in FindAllBrowserTypes(), (
        'Please add %s to cros_browser_finder.FindAllBrowserTypes()' %
        browser_type)
    del finder_options
    self._platform = cros_platform
    self._platform_backend = (
        cros_platform._platform_backend)  # pylint: disable=protected-access
    self._is_guest = is_guest

    self._existing_minidump_dir = None
    # There's no way to automatically determine the build directory, as
    # unlike Android, we're not responsible for installing the browser. If
    # we're running on a bot, then the CrOS wrapper script will set this
    # accordingly, and normal users can pass --chromium-output-dir to have
    # this set in browser_options.
    self._build_dir = os.environ.get('CHROMIUM_OUTPUT_DIR')

  def __repr__(self):
    return 'PossibleCrOSBrowser(browser_type=%s)' % self.browser_type

  @property
  def browser_directory(self):
    result = self._platform_backend.cri.GetChromeProcess()
    if result and 'path' in result:
      return posixpath.dirname(result['path'])
    return None

  @property
  def profile_directory(self):
    return '/home/chronos/Default'

  def _InitPlatformIfNeeded(self):
    pass

  def _GetPathsForOsPageCacheFlushing(self):
    return [self.profile_directory, self.browser_directory]

  def SetUpEnvironment(self, browser_options):
    super().SetUpEnvironment(browser_options)

    # Copy extensions to temp directories on the device.
    # Note that we also perform this copy locally to ensure that
    # the owner of the extensions is set to chronos.
    cri = self._platform_backend.cri
    for extension in self._browser_options.extensions_to_load:
      extension_dir = cri.RunCmdOnDevice(
          ['mktemp', '-d', '/tmp/extension_XXXXX'])[0].rstrip()
      # TODO(crbug.com/807645): We should avoid having mutable objects
      # stored within the browser options.
      extension.local_path = posixpath.join(
          extension_dir, os.path.basename(extension.path))
      cri.PushFile(extension.path, extension_dir)
      cri.Chown(extension_dir)

    # Move any existing crash dumps temporarily so that they don't get deleted.
    self._existing_minidump_dir = (
        '/tmp/existing_minidumps_%s' % _GetRandomHash())
    cri.RunCmdOnDevice(
        ['mv', self._CROS_MINIDUMP_DIR, self._existing_minidump_dir])

    if self._browser_options.clear_enterprise_policy:
      cri.StopUI()
      cri.RmRF('/var/lib/devicesettings/*')

      local_state_path = '/home/chronos/Local State'
      if self._browser_options.dont_override_profile:
        # Ash keeps track of Lacros profile migration using local state prefs.
        # Clobbering local state with Lacros enabled will re-initiate Lacros
        # profile migration and effectively wipe the existing profile. In
        # addition, the profile migration involves restarting Ash and
        # CrOSBrowserBackend will lose its DevTools connection.
        #
        # Without knowing what clear_enterprise_policy specifically seeks to
        # remove from local state, our solution here is to remove everything
        # except the Lacros migration bits.
        local_state = json.loads(cri.GetFileContents(local_state_path))
        for key in list(local_state.keys()):
          if not key == 'lacros':
            del local_state[key]
        cri.PushContents(json.dumps(local_state, separators=(',',':')),
                         local_state_path)
      else:
        cri.RmRF(local_state_path)

    def browser_ready():
      return cri.GetChromePid() is not None

    cri.RestartUI()
    py_utils.WaitFor(browser_ready, timeout=20)

    # Delete test user's cryptohome vault (user data directory).
    if not self._browser_options.dont_override_profile:
      cri.RunCmdOnDevice(['cryptohome', '--action=remove', '--force',
                          '--user=%s' % self._browser_options.username])

  def _TearDownEnvironment(self):
    cri = self._platform_backend.cri
    for extension in self._browser_options.extensions_to_load:
      cri.RmRF(posixpath.dirname(extension.local_path))

    # Move back any dumps that existed before we started the test.
    cri.RmRF(self._CROS_MINIDUMP_DIR)
    cri.RunCmdOnDevice(
        ['mv', self._existing_minidump_dir, self._CROS_MINIDUMP_DIR])

  def Create(self):
    # Init the LocalFirstBinaryManager if this is the first time we're creating
    # a browser.
    if local_first_binary_manager.LocalFirstBinaryManager.NeedsInit():
      local_first_binary_manager.LocalFirstBinaryManager.Init(
          self._build_dir, None, 'linux', platform.machine(),
          # TODO(crbug.com/1084334): Remove crashpad_database_util once the
          # locally compiled version works.
          ignored_dependencies=['crashpad_database_util'])

    startup_args = self.GetBrowserStartupArgs(self._browser_options)

    os_browser_backend = cros_browser_backend.CrOSBrowserBackend(
        self._platform_backend, self._browser_options,
        self.browser_directory, self.profile_directory,
        self._is_guest, self._DEFAULT_CHROME_ENV,
        build_dir=self._build_dir,
        enable_tracing=self._app_type != 'lacros-chrome')

    if self._browser_options.create_browser_with_oobe:
      return cros_browser_with_oobe.CrOSBrowserWithOOBE(
          os_browser_backend, self._platform_backend, startup_args)
    os_browser_backend.Start(startup_args)

    if self._app_type == 'lacros-chrome':
      lacros_chrome_browser_backend = (
          lacros_browser_backend.LacrosBrowserBackend(
              self._platform_backend,
              self._browser_options,
              self.browser_directory,
              self.profile_directory,
              self._DEFAULT_CHROME_ENV,
              os_browser_backend,
              build_dir=self._build_dir))
      return browser.Browser(
          lacros_chrome_browser_backend, self._platform_backend, startup_args)
    return browser.Browser(os_browser_backend, self._platform_backend,
                           startup_args)

  def GetBrowserStartupArgs(self, browser_options):
    startup_args = chrome_startup_args.GetFromBrowserOptions(browser_options)

    startup_args.extend(chrome_startup_args.GetReplayArgs(
        self._platform_backend.network_controller_backend))

    vmodule = ','.join('%s=2' % pattern for pattern in [
        '*/chromeos/net/*',
        '*/chromeos/login/*',
        'chrome_browser_main_posix'])

    startup_args.extend([
        '--enable-smooth-scrolling',
        '--enable-threaded-compositing',
        # Allow devtools to connect to chrome.
        '--remote-debugging-port=0',
        # Open a maximized window.
        '--start-maximized',
        # Disable sounds.
        '--ash-disable-system-sounds',
        # Skip user image selection screen, and post login screens.
        '--oobe-skip-postlogin',
        # Enable OOBE test API
        '--enable-oobe-test-api',
        # Force launch browser
        '--force-launch-browser',
        # Debug logging.
        '--vmodule=%s' % vmodule,
        # Enable crash dumping.
        '--enable-crash-reporter-for-testing',
        # Do not trigger powerwash if tests misbehave.
        '--cryptohome-ignore-cleanup-ownership-for-testing',
    ])

    if self._app_type == 'lacros-chrome':
      startup_args.extend(['--lacros-mojo-socket-for-testing=/tmp/lacros.sock'])

    if browser_options.mute_audio:
      startup_args.append('--mute-audio')

    if not browser_options.expect_policy_fetch:
      startup_args.append('--allow-failed-policy-fetch-for-test')

    if browser_options.disable_gaia_services:
      startup_args.append('--disable-gaia-services')

    trace_config_file = (self._platform_backend.tracing_controller_backend
                         .GetChromeTraceConfigFile())
    if trace_config_file:
      startup_args.append('--trace-config-file=%s' % trace_config_file)

    return startup_args

  def SupportsOptions(self, browser_options):
    return (len(browser_options.extensions_to_load) == 0) or not self._is_guest

  def UpdateExecutableIfNeeded(self):
    pass

def SelectDefaultBrowser(possible_browsers):
  if cros_device.IsRunningOnCrOS():
    for b in possible_browsers:
      if b.browser_type == 'system':
        return b
  return None


def CanFindAvailableBrowsers(finder_options):
  return (cros_device.IsRunningOnCrOS() or finder_options.fetch_cros_remote
          or finder_options.remote or linux_based_interface.HasSSH())


def FindAllBrowserTypes():
  return [
      'cros-chrome',
      'cros-chrome-guest',
      'lacros-chrome',
      'system',
      'system-guest',
  ]


def FindAllAvailableBrowsers(finder_options, device):
  """Finds all available CrOS browsers, locally and remotely."""
  browsers = []
  if not isinstance(device, cros_device.CrOSDevice):
    return browsers

  if cros_device.IsRunningOnCrOS():
    browsers = [
        PossibleCrOSBrowser(
            'system',
            finder_options,
            platform_module.GetHostPlatform(),
            is_guest=False),
        PossibleCrOSBrowser(
            'system-guest',
            finder_options,
            platform_module.GetHostPlatform(),
            is_guest=True)
    ]

  # Check ssh
  try:
    plat = platform_module.GetPlatformForDevice(device, finder_options)
  except linux_based_interface.LoginException as ex:
    if isinstance(ex, linux_based_interface.KeylessLoginRequiredException):
      logging.warning('Could not ssh into %s. Your device must be configured',
                      finder_options.remote)
      logging.warning('to allow passwordless login as root.')
      logging.warning('For a test-build device, pass this to your script:')
      logging.warning('   --identity $(CHROMITE)/ssh_keys/testing_rsa')
      logging.warning('')
      logging.warning('For a developer-mode device, the steps are:')
      logging.warning(' - Ensure you have an id_rsa.pub (etc) on this computer')
      logging.warning(' - On the chromebook:')
      logging.warning('   -  Control-Alt-T; shell; sudo -s')
      logging.warning('   -  openssh-server start')
      logging.warning('   -  scp <this machine>:.ssh/id_rsa.pub /tmp/')
      logging.warning('   -  mkdir /root/.ssh')
      logging.warning('   -  chown go-rx /root/.ssh')
      logging.warning('   -  cat /tmp/id_rsa.pub >> /root/.ssh/authorized_keys')
      logging.warning('   -  chown 0600 /root/.ssh/authorized_keys')
    raise browser_finder_exceptions.BrowserFinderException(str(ex))

  browsers.extend([
      PossibleCrOSBrowser(
          'cros-chrome', finder_options, plat, is_guest=False),
      PossibleCrOSBrowser(
          'lacros-chrome', finder_options, plat, is_guest=False),
      PossibleCrOSBrowser(
          'cros-chrome-guest', finder_options, plat, is_guest=True)
  ])
  return browsers


def _GetRandomHash():
  return '%08x' % random.getrandbits(32)
