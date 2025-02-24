# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import os
import posixpath
import shutil
import time

import py_utils
from py_utils import exc_util

from telemetry.core import exceptions
from telemetry import decorators
from telemetry.internal.backends.chrome import chrome_browser_backend
from telemetry.internal.backends.chrome import cros_minidump_symbolizer
from telemetry.internal.backends.chrome import minidump_finder
from telemetry.internal.backends.chrome import misc_web_contents_backend
from telemetry.internal.results import artifact_logger
from telemetry.internal.util import format_for_logging


class CrOSBrowserBackend(chrome_browser_backend.ChromeBrowserBackend):
  def __init__(self, cros_platform_backend, browser_options,
               browser_directory, profile_directory, is_guest, env,
               build_dir=None, enable_tracing=True):
    """
    Args:
      cros_platform_backend: The cros_platform_backend.CrOSPlatformBackend
          instance to use.
      browser_options: The browser_options.BrowserOptions instance to use.
      browser_directory: A string containing the path to the directory on the
          device where the browser is installed.
      enable_tracing: Defines if a tracing_client is created.
      profile_directory: A string containing a path to the directory on the
          device to store browser profile information in.
      is_guest: A boolean indicating whether the browser is being run in guest
          mode or not.
      env: A list of strings containing environment variables to start the
          browser with.
      build_dir: A string containing a path to the directory on the host that
          the browser was built in, for finding debug artifacts. Can be None if
          the browser was not locally built, or the directory otherwise cannot
          be determined.
    """
    assert browser_options.IsCrosBrowserOptions()
    super().__init__(
        cros_platform_backend,
        browser_options=browser_options,
        browser_directory=browser_directory,
        enable_tracing=enable_tracing,
        profile_directory=profile_directory,
        supports_extensions=not is_guest,
        supports_tab_control=True,
        build_dir=build_dir)
    self._is_guest = is_guest
    self._cri = cros_platform_backend.cri
    self._env = env
    self._started = False

  @property
  def log_file_path(self):
    return None

  def _GetDevToolsActivePortPath(self):
    return '/home/chronos/DevToolsActivePort'

  def _FindDevToolsPortAndTarget(self):
    devtools_file_path = self._GetDevToolsActivePortPath()
    # GetFileContents may rise IOError or OSError, the caller will retry.
    lines = self._cri.GetFileContents(devtools_file_path).splitlines()
    if not lines:
      raise EnvironmentError('DevTools file empty')

    devtools_port = int(lines[0])
    browser_target = lines[1] if len(lines) >= 2 else None
    return devtools_port, browser_target

  def _FindUIDevtoolsPort(self):
    devtools_file_path = '/home/chronos/UIDevToolsActivePort'
    # GetFileContents may rise IOError or OSError, the caller will retry.
    lines = self._cri.GetFileContents(devtools_file_path).splitlines()
    if not lines:
      raise EnvironmentError('UIDevTools file empty')

    devtools_port = int(lines[0])
    return devtools_port

  def GetPid(self):
    return self._cri.GetChromePid()

  def Start(self, startup_args):
    if self._started:
      return
    self._started = True

    self._cri.OpenConnection()
    # Remove the stale file with the devtools port / browser target
    # prior to restarting chrome.
    self._cri.RmRF(self._GetDevToolsActivePortPath())

    self._dump_finder = minidump_finder.MinidumpFinder(
        self._platform_backend.platform.GetOSName(),
        self._platform_backend.platform.GetArchName())

    # Escape all commas in the startup arguments we pass to Chrome
    # because dbus-send delimits array elements by commas
    startup_args = [a.replace(',', '\\,') for a in startup_args]

    # Restart Chrome with the login extension and remote debugging.
    pid = self.GetPid()
    logging.info('Restarting Chrome (pid=%d) with remote port', pid)
    args = ['dbus-send', '--system', '--type=method_call',
            '--dest=org.chromium.SessionManager',
            '/org/chromium/SessionManager',
            'org.chromium.SessionManagerInterface.EnableChromeTesting',
            'boolean:true',
            'array:string:"%s"' % ','.join(startup_args),
            'array:string:"%s"' % ','.join(self._env)]
    formatted_args = format_for_logging.ShellFormat(
        args, trim=self.browser_options.trim_logs)
    logging.info('Starting Chrome: %s', formatted_args)
    self._cri.RunCmdOnDevice(args)

    # Wait for new chrome and oobe.
    py_utils.WaitFor(lambda: pid != self.GetPid(), 15)
    self.BindDevToolsClient()
    py_utils.WaitFor(lambda: self.oobe_exists, 30)

    if self.browser_options.auto_login:
      if self._is_guest:
        pid = self.GetPid()
        self.oobe.NavigateGuestLogin()
        # Guest browsing shuts down the current browser and launches an
        # incognito browser in a separate process, which we need to wait for.
        try:
          py_utils.WaitFor(lambda: pid != self.GetPid(), 15)

          # Also make sure we reconnect the devtools client to the new browser
          # process. It's important to do this before waiting for _IsLoggedIn,
          # otherwise the devtools connection check will still try to reach the
          # older DevTools agent (and fail to do so).
          self.BindDevToolsClient()
        except py_utils.TimeoutException:
          self._RaiseOnLoginFailure(
              'Failed to restart browser in guest mode (pid %d).' % pid)

      elif self.browser_options.gaia_login:
        self.oobe.NavigateGaiaLogin(self._username, self._password)
      else:
        # Wait for few seconds(the time of password typing) to have mini ARC
        # container up and running. Default is 0.
        time.sleep(self.browser_options.login_delay)
        # crbug.com/976983.
        retries = 3
        while True:
          try:
            self.oobe.NavigateFakeLogin(
                self._username, self._password, self._gaia_id,
                not self.browser_options.disable_gaia_services)
            break
          except py_utils.TimeoutException:
            logging.error('TimeoutException %d', retries)
            retries -= 1
            if not retries:
              raise

      try:
        self._WaitForLogin()
      except py_utils.TimeoutException:
        self._RaiseOnLoginFailure('Timed out going through login screen. '
                                  + self._GetLoginStatus())

    logging.info('Browser is up!')

  def Background(self):
    raise NotImplementedError

  @exc_util.BestEffort
  def Close(self):
    super().Close()

    if self._cri:
      self._cri.RestartUI() # Logs out.
      py_utils.WaitFor(lambda: not self._IsCryptohomeMounted(), 180)
      self._cri.CloseConnection()

    self._cri = None

    if self._tmp_minidump_dir:
      shutil.rmtree(self._tmp_minidump_dir, ignore_errors=True)
      self._tmp_minidump_dir = None

  def IsBrowserRunning(self):
    if not self._cri:
      return False
    return bool(self.GetPid())

  def GetStandardOutput(self):
    return 'Cannot get standard output on CrOS'

  def PullMinidumps(self):
    if self._cri:
      self._cri.PullDumps(self._tmp_minidump_dir)
    else:
      logging.error(
          'Attempted to pull minidumps without CrOSInterface. Either the '
          'browser is already closed or was never started.')

  def SymbolizeMinidump(self, minidump_path):
    return self._SymbolizeMinidump(minidump_path)

  def CollectDebugData(self, log_level):
    """Collects various information that may be useful for debugging.

    In addition to data captured by the parent class' implementation, also
    captures:
    1. Chrome logs
    2. ChromeOS UI logs

    Args:
      log_level: The logging level to use from the logging module, e.g.
          logging.ERROR.

    Returns:
      A debug_data.DebugData object containing the collected data.
    """
    self._CollectBrowserLogs(log_level)
    self._CollectUiLogs(log_level)
    return super().CollectDebugData(log_level)

  def _CollectBrowserLogs(self, log_level):
    """Helper function to handle the browser log part of CollectDebugData.

    Attempts to retrieve the current and previous browser logs, merge them, and
    save the result as an artifact.

    Args:
      log_level: The logging level to use from the logging module, e.g.
          logging.ERROR.
    """
    # /var/log/chrome/chrome is the log for the current browser, but in case
    # there's something useful in the previous browser's logs, merge chrome
    # and chrome.PREVIOUS.
    try:
      current_log = self._cri.GetFileContents('/var/log/chrome/chrome')
    except OSError:
      logging.log(log_level, 'Unexpectedly did not find browser log')
      return
    current_log = '#### Current Chrome Log ####\n\n%s' % current_log
    try:
      previous_log = self._cri.GetFileContents(
          '/var/log/chrome/chrome.PREVIOUS')
    except OSError:
      # This is expected if this is the first browser launch on this device.
      previous_log = 'Did not find a previous Chrome log.'
    merged_log = '%s\n\n#### Previous Chrome Log ####\n\n%s' % (current_log,
                                                                previous_log)
    artifact_name = posixpath.join(
        'browser_logs', 'browser_log-%s' % artifact_logger.GetTimestampSuffix())
    logging.log(log_level, 'Saving browser log as artifact %s', artifact_name)
    artifact_logger.CreateArtifact(artifact_name, merged_log)

  def _CollectUiLogs(self, log_level):
    """Helper function to handle the UI log part of CollectDebugData.

    Attempts to retrieve the current UI log and save it as an artifact.

    Args:
      log_level: The logging level to use from the logging module, e.g.
          logging.ERROR.
    """
    # Unlike the browser logs, there is no .PREVIOUS version, so we can only
    # easily get the most recent UI log.
    try:
      ui_log = self._cri.GetFileContents('/var/log/ui/ui.LATEST')
    except OSError:
      logging.log(log_level, 'Unexpectedly did not find UI log')
      return
    artifact_name = posixpath.join(
        'ui_logs', 'ui_log-%s' % artifact_logger.GetTimestampSuffix())
    logging.log(log_level, 'Saving UI log as artifact %s', artifact_name)
    artifact_logger.CreateArtifact(artifact_name, ui_log)

  @property
  def screenshot_timeout(self):
    # Screenshots fail when the screen is off, and we can flakily attempt to
    # capture screenshots on failure when the screen is off. So, retry for a
    # while if we run into that.
    return 15

  @property
  def supports_overview_mode(self): # pylint: disable=invalid-name
    return True

  def EnterOverviewMode(self, timeout):
    self.devtools_client.window_manager_backend.EnterOverviewMode(timeout)

  def ExitOverviewMode(self, timeout):
    self.devtools_client.window_manager_backend.ExitOverviewMode(timeout)

  @property
  @decorators.Cache
  def misc_web_contents_backend(self):
    """Access to chrome://oobe/login page."""
    return misc_web_contents_backend.MiscWebContentsBackend(self)

  @property
  def oobe(self):
    return self.misc_web_contents_backend.GetOobe()

  @property
  def oobe_exists(self):
    return self.misc_web_contents_backend.oobe_exists

  @property
  def _username(self):
    return self.browser_options.username

  @property
  def _password(self):
    return self.browser_options.password

  @property
  def _gaia_id(self):
    return self.browser_options.gaia_id

  def _IsCryptohomeMounted(self):
    username = '$guest' if self._is_guest else self._username
    return self._cri.IsCryptohomeMounted(username, self._is_guest)

  def _GetLoginStatus(self):
    """Returns login status. If logged in, empty string is returned."""
    status = ''
    if not self._IsCryptohomeMounted():
      status += 'Cryptohome not mounted. '
    if not self.HasDevToolsConnection():
      status += 'Browser didn\'t launch. '
    if self.oobe_exists:
      status += 'OOBE not dismissed.'
    return status

  def _IsLoggedIn(self):
    """Returns True if cryptohome has mounted, the browser is
    responsive to devtools requests, and the oobe has been dismissed."""
    return not self._GetLoginStatus()

  def _WaitForLogin(self):
    # Wait for cryptohome to mount.
    py_utils.WaitFor(self._IsLoggedIn, 900)

    # Wait for extensions to load.
    if self._supports_extensions:
      self._WaitForExtensionsToLoad()

  def _RaiseOnLoginFailure(self, error):
    if self._platform_backend.CanTakeScreenshot():
      self._cri.TakeScreenshotWithPrefix('login-screen')
    raise exceptions.LoginException(error)

  def _SymbolizeMinidump(self, minidump_path):
    """Symbolizes the given minidump.

    Args:
      minidump_path: the path to the minidump to symbolize

    Return:
      A tuple (valid, output). |valid| is True if the minidump was symbolized,
      otherwise False. |output| contains an error message if |valid| is False,
      otherwise it contains the symbolized minidump.
    """
    # TODO(https://crbug.com/994267): Make the minidump being symbolized
    # available as an artifact.
    stack = self._GetStackFromMinidump(minidump_path)
    if not stack:
      error_message = ('Failed to symbolize minidump.')
      return (False, error_message)

    self._symbolized_minidump_paths.add(minidump_path)
    return (True, stack)

  def _GetStackFromMinidump(self, minidump):
    """Gets the stack trace from the given minidump.

    Args:
      minidump: the path to the minidump on disk

    Returns:
      None if the stack could not be retrieved for some reason, otherwise a
      string containing the stack trace.
    """
    dump_symbolizer = cros_minidump_symbolizer.CrOSMinidumpSymbolizer(
        self._dump_finder, self.build_dir,
        symbols_dir=self._CreateExecutableUniqueDirectory('chrome_symbols_'))
    return dump_symbolizer.SymbolizeMinidump(minidump)

  def _GetBrowserExecutablePath(self):
    if self.build_dir:
      possible_path = os.path.join(self.build_dir, 'chrome')
      if os.path.isfile(possible_path):
        return possible_path
    return None
