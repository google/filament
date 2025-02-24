# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import absolute_import
import logging
import posixpath
import shutil
import time
import six.moves._thread  # pylint: disable=import-error

import py_utils
from py_utils import exc_util

from telemetry import decorators
from telemetry.core import debug_data
from telemetry.internal.backends.chrome import chrome_browser_backend
from telemetry.internal.backends.chrome import minidump_finder

class LacrosBrowserBackend(chrome_browser_backend.ChromeBrowserBackend):
  def __init__(self, cros_platform_backend, browser_options,
               browser_directory, profile_directory, env,
               cros_browser_backend, build_dir=None):
    """
    Args:
      cros_platform_backend: The cros_platform_backend.CrOSPlatformBackend
          instance to use.
      browser_options: The browser_options.BrowserOptions instance to use.
      browser_directory: A string containing the path to the directory on the
          device where the browser is installed.
      profile_directory: A string containing a path to the directory on the
          device to store browser profile information in.
      env: A list of strings containing environment variables to start the
          browser with.
      cros_browser_backend: The CrOs browser Lacros is running on top of.
          Some actions (e.g. Close) are further delegated to this.
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
        profile_directory=profile_directory,
        supports_extensions=True,
        supports_tab_control=True,
        build_dir=build_dir)
    self._cri = cros_platform_backend.cri
    self._env = env
    self._devtools_client_os = None
    self._devtools_port_path = self._GetDevToolsActivePortPath()
    self._cros_browser_backend = cros_browser_backend
    self._is_browser_running = False

  @property
  def log_file_path(self):
    return None

  def _GetDevToolsActivePortPath(self):
    return '/usr/local/lacros-chrome/user_data/DevToolsActivePort'

  def _RunCommandAndLog(self, cmd):
    results = self._cri.RunCmdOnDevice(cmd)
    logging.info("stdout: " + results[0])
    logging.info("stderr: " + results[1])

  def _IsDevtoolsUp(self):
    return self._cri.FileExistsOnDevice(self._GetDevToolsActivePortPath())

  # TODO(crbug.com/1148868): Share this and others with CrOSBrowserBackend.
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
    devtools_file_path = posixpath.join(
        '/', 'usr', 'local', 'lacros-chrome', 'user_data',
        'UIDevToolsActivePort')
    # GetFileContents may raise IOError or OSError, the caller will retry.
    lines = self._cri.GetFileContents(devtools_file_path).splitlines()
    if not lines:
      raise EnvironmentError('UIDevTools file empty')

    devtools_port = int(lines[0])
    return devtools_port

  def GetPid(self):
    return self._cri.GetChromePid()

  def _ReformatArg(self, startup_args, arg_name):
    arg_str = '--' + arg_name + '='
    for i, arg in enumerate(startup_args):
      if arg_str in arg:
        new_arg = arg
        new_arg = new_arg.replace(arg_str, arg_str + "'")
        new_arg = new_arg + "'"
        new_arg = new_arg.replace(';', '\\;')
        new_arg = new_arg.replace(',', '\\,')
        startup_args[i] = new_arg

  def _LaunchLacrosChromeHelper(self, startup_args):
    self._cri.RunCmdOnDevice(['cp', '/usr/local/lacros-chrome/chrome',
                              '/usr/local/lacros-chrome/lacros-chrome'])

    # Some args need escaping, etc.
    self._ReformatArg(startup_args, 'enable-features')
    self._ReformatArg(startup_args, 'disable-features')
    self._ReformatArg(startup_args, 'force-fieldtrials')
    self._ReformatArg(startup_args, 'force-fieldtrial-params')
    self._ReformatArg(startup_args, 'proxy-bypass-list')
    self._ReformatArg(startup_args, 'user-agent')

    logging.info('Starting Chrome w/ Args: %s', startup_args)

    def _Launch():
      # This will block until the launched browser is closed.
      self._RunCommandAndLog(
          ['EGL_PLATFORM=surfaceless',
           'XDG_RUNTIME_DIR=/run/chrome',
           'python',
           '/usr/local/bin/mojo_connection_lacros_launcher.py',
           '-s', '/tmp/lacros.sock',
           '/usr/local/lacros-chrome/lacros-chrome',
           '--ozone-platform=wayland',
           '--user-data-dir=/usr/local/lacros-chrome/user_data',
           '--enable-gpu-rasterization',
           '--enable-oop-rasterization',
           '--lang=en-US',
           '--breakpad-dump-location=/usr/local/lacros-chrome/',
           '--no-sandbox',
           '--trace-smb-size=32768',
           # Add LLVM profile output arg regardless of whether it's used.
           # For PGO enabled builds, profiles are written to /tmp/profraw
           # on the device. See crrev/c/4358910 for definition on arg.
           '--llvm-profile-file=/tmp/default-%2m.profraw'] + startup_args)
      # This will only exist if launch was successful.
      return self._IsDevtoolsUp()

    # TODO(crbug/1148534) - Launch only works sporadically.
    # This will return either when the timeout is hit or the test closes Lacros,
    # whichever is *later*
    py_utils.WaitFor(_Launch, 40)
    self._is_browser_running = False

  def LaunchLacrosChrome(self, startup_args):
    six.moves._thread.start_new_thread(
        self._LaunchLacrosChromeHelper, (startup_args,))
    py_utils.WaitFor(self._IsDevtoolsUp, 40)
    self._is_browser_running = self._IsDevtoolsUp()
    # TODO(crbug/1150455) - Find another condtion to wait on.
    time.sleep(1)
    print('Is Lacros up? ' + str(self._is_browser_running))

  def Start(self, startup_args):
    self._cri.OpenConnection()
    # Remove the stale file with the devtools port / browser target
    # prior to restarting chrome.
    self._cri.RmRF(self._GetDevToolsActivePortPath())

    self._dump_finder = minidump_finder.MinidumpFinder(
        self.browser.platform.GetOSName(), self.browser.platform.GetArchName())

    self.LaunchLacrosChrome(startup_args)
    if not self._is_browser_running:
      raise EnvironmentError('Unable to launch Lacros. If this is a local '
       'run, did you install Lacros via deploy_chrome? If not, build and '
       'deploy Lacros by following the instructions at '
       'https://docs.google.com/document/d/1gkc_df4pv0OdVqRTz49kTkSsIybTAU_b'
       'KfjFSWNcqnk/edit#bookmark=kix.edfe7j8hizjs '
       '(Googlers only).')
    self.BindDevToolsClient()

  def Background(self):
    raise NotImplementedError

  @exc_util.BestEffort
  def Close(self):
    super().Close()

    if self._tmp_minidump_dir:
      shutil.rmtree(self._tmp_minidump_dir, ignore_errors=True)
      self._tmp_minidump_dir = None

    # Underlying CrOS browser is responsible for closing the cri
    self._cros_browser_backend.Close()

  def IsBrowserRunning(self):
    return self._is_browser_running

  def GetStandardOutput(self):
    return 'Cannot get standard output on Lacros'

  def PullMinidumps(self):
    if self._cri:
      self._cri.PullDumps(self._tmp_minidump_dir)
    else:
      logging.error(
          'Attempted to pull minidumps without CrOSInterface. Either the '
          'browser is already closed or was never started.')

  def SymbolizeMinidump(self, minidump_path):
    return self._cros_browser_backend.SymbolizeMinidump(minidump_path)

  def _GetBrowserExecutablePath(self):
    # pylint: disable=protected-access
    return self._cros_browser_backend._GetBrowserExecutablePath()
    # pylint: enable=protected-access

  def CollectDebugData(self, log_level):
    """Collects various information that may be useful for debugging.

    Args:
      log_level: The logging level to use from the logging module, e.g.
          logging.ERROR.

    Returns:
      A debug_data.DebugData object containing the collected data.
    """
    # TODO(crbug.com/1148528): Pull Lacros data.
    return debug_data.DebugData()

  @property
  def screenshot_timeout(self):
    # Screenshots fail when the screen is off, and we can flakily attempt to
    # capture screenshots on failure when the screen is off. So, retry for a
    # while if we run into that.
    return 15

  @property
  def supports_overview_mode(self): # pylint: disable=invalid-name
    return True

  @property
  def devtools_window_manager_backend(self):
    # pylint: disable=protected-access
    return self._cros_browser_backend._devtools_client.window_manager_backend
    # pylint: enable=protected-access

  def EnterOverviewMode(self, timeout):
    self.devtools_window_manager_backend.EnterOverviewMode(timeout)

  def ExitOverviewMode(self, timeout):
    self.devtools_window_manager_backend.ExitOverviewMode(timeout)

  @property
  @decorators.Cache
  def misc_web_contents_backend(self):
    """Access to chrome://oobe/login page."""
    return self._cros_browser_backend.misc_web_contents_backend

  @property
  def oobe(self):
    return self._cros_browser_backend.misc_web_contents_backend.GetOobe()

  @property
  def oobe_exists(self):
    return self._cros_browser_backend.misc_web_contents_backend.oobe_exists
