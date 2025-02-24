# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import os
import posixpath
import shutil

from py_utils import exc_util

from telemetry.core import exceptions
from telemetry.internal.platform import android_platform_backend as \
  android_platform_backend_module
from telemetry.internal.backends.chrome import android_minidump_symbolizer
from telemetry.internal.backends.chrome import chrome_browser_backend
from telemetry.internal.backends.chrome import minidump_finder
from telemetry.internal.browser import user_agent
from telemetry.internal.results import artifact_logger

from devil.android import app_ui
from devil.android import device_signal
from devil.android.sdk import intent
from devil.android.sdk import version_codes


class AndroidBrowserBackend(chrome_browser_backend.ChromeBrowserBackend):
  """The backend for controlling a browser instance running on Android."""
  DEBUG_ARTIFACT_PREFIX = 'android_debug_info'

  def __init__(self, android_platform_backend, finder_options,
               browser_directory, profile_directory, backend_settings,
               build_dir=None, local_apk_path=None):
    """
    Args:
      android_platform_backend: The
          android_platform_backend.AndroidPlatformBackend instance to use.
      browser_options: The browser_options.BrowserOptions instance to use.
      browser_directory: A string containing the path to the directory on the
          device where the browser is installed.
      profile_directory: A string containing a path to the directory on the
          device to store browser profile information in.
      backend_settings: The
          android_browser_backend_settings.AndroidBrowserBackendSettings
          instance to use.
      build_dir: A string containing a path to the directory on the host that
          the browser was built in, for finding debug artifacts. Can be None if
          the browser was not locally built, or the directory otherwise cannot
          be determined.
      local_apk_path: A string containing a path to the APK for the browser. Can
          be None if the browser was not locally built.
    """
    assert isinstance(android_platform_backend,
                      android_platform_backend_module.AndroidPlatformBackend)
    super().__init__(
        android_platform_backend,
        browser_options=finder_options.browser_options,
        browser_directory=browser_directory,
        profile_directory=profile_directory,
        supports_extensions=False,
        supports_tab_control=backend_settings.supports_tab_control,
        build_dir=build_dir)
    self._backend_settings = backend_settings

    # Initialize fields so that an explosion during init doesn't break in Close.
    self._saved_sslflag = ''
    self._app_ui = None
    self._browser_package = None
    self._finder_options = finder_options
    self._local_apk_path = local_apk_path

    # Set the debug app if needed.
    self.platform_backend.SetDebugApp(self.browser_package)

  @property
  def log_file_path(self):
    return None

  @property
  def device(self):
    return self.platform_backend.device

  @property
  def supports_app_ui_interactions(self):
    return True

  def GetAppUi(self):
    if self._app_ui is None:
      self._app_ui = app_ui.AppUi(self.device, package=self.browser_package)
    return self._app_ui

  def _StopBrowser(self):
    # Note: it's important to stop and _not_ kill the browser app, since
    # stopping also clears the app state in Android's activity manager.
    self.platform_backend.StopApplication(self.browser_package)

  def Start(self, startup_args):
    assert not startup_args, (
        'Startup arguments for Android should be set during '
        'possible_browser.SetUpEnvironment')
    self._dump_finder = minidump_finder.MinidumpFinder(
        self.browser.platform.GetOSName(), self.browser.platform.GetArchName())
    user_agent_dict = user_agent.GetChromeUserAgentDictFromType(
        self.browser_options.browser_user_agent_type)
    activity = self._backend_settings.GetActivityNameForSdk(
        self.platform_backend.device.build_version_sdk)
    action = self._backend_settings.GetActionForSdk(
        self.platform_backend.device.build_version_sdk)
    self.device.StartActivity(intent.Intent(package=self.browser_package,
                                            activity=activity,
                                            action=action,
                                            data='about:blank',
                                            category=None,
                                            extras=user_agent_dict),
                              blocking=True)
    try:
      self.BindDevToolsClient()
    except:
      self.Close()
      raise

  def BindDevToolsClient(self):
    super().BindDevToolsClient()
    package = self.devtools_client.GetVersion().get('Android-Package')
    if package is None:
      logging.warning('Could not determine package name from DevTools client.')
    elif package == self.browser_package:
      logging.info('Successfully connected to %s DevTools client', package)
    else:
      raise exceptions.BrowserGoneException(
          self.browser, 'Expected connection to %s but got %s.' % (
              self.browser_package, package))

  def _FindDevToolsPortAndTarget(self):
    devtools_port = self._backend_settings.GetDevtoolsRemotePort(
        self.device, self.browser_package)
    browser_target = None  # Use default
    return devtools_port, browser_target

  def Foreground(self):
    package = self.browser_package
    activity = self._backend_settings.GetActivityNameForSdk(
        self.platform_backend.device.build_version_sdk)
    action = self._backend_settings.GetActionForSdk(
        self.platform_backend.device.build_version_sdk)
    self.device.StartActivity(intent.Intent(
        package=package,
        activity=activity,
        action=action,
        flags=[intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED]),
                              blocking=False)
    # TODO(crbug.com/601052): The following waits for any UI node for the
    # package launched to appear on the screen. When the referenced bug is
    # fixed, remove this workaround and just switch blocking above to True.
    try:
      app_ui.AppUi(self.device).WaitForUiNode(package=package)
    except Exception as e:
      raise exceptions.BrowserGoneException(
          self.browser,
          'Timed out waiting for browser to come back foreground.') from e

  def Background(self):
    package = 'org.chromium.push_apps_to_background'
    activity = package + '.PushAppsToBackgroundActivity'
    self.device.StartActivity(
        intent.Intent(
            package=package,
            activity=activity,
            action=None,
            flags=[intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED]),
        blocking=True)

  def ForceJavaHeapGarbageCollection(self):
    # Send USR1 signal to force GC on Chrome processes forked from Zygote.
    # (c.f. crbug.com/724032)
    self.device.KillAll(
        self.browser_package,
        exact=False,  # Send signal to children too.
        signum=device_signal.SIGUSR1,
        as_root=True)

  @property
  def processes(self):
    try:
      zygotes = self.device.ListProcesses('zygote')
      zygote_pids = set(p.pid for p in zygotes)
      assert zygote_pids, 'No Android zygote found'
      processes = self.device.ListProcesses(self.browser_package)
      return [p for p in processes if p.ppid in zygote_pids]
    except Exception as exc:
      # Re-raise as an AppCrashException to get further diagnostic information.
      # In particular we also get the values of all local variables above.
      raise exceptions.AppCrashException(
          self.browser, 'Error getting browser PIDs: %s' % exc)

  def GetPid(self):
    browser_processes = self._GetBrowserProcesses()
    assert len(browser_processes) <= 1, (
        'Found too many browsers: %r' % browser_processes)
    if not browser_processes:
      raise exceptions.BrowserGoneException(self.browser)
    return browser_processes[0].pid

  def _GetBrowserProcesses(self):
    """Return all possible browser processes."""
    package = self.browser_package
    return [p for p in self.processes if p.name == package]

  @property
  def browser_package(self):
    if not self._browser_package:
      self._browser_package = self._backend_settings.package
      if self._backend_settings.IsWebView():
        self._browser_package = (
            self._backend_settings.
            GetEmbedderPackageName(self._finder_options))
    return self._browser_package

  @property
  def activity(self):
    return self._backend_settings.activity

  @exc_util.BestEffort
  def Close(self):
    if os.getenv('CHROME_PGO_PROFILING'):
      self.devtools_client.DumpProfilingDataOfAllProcesses(timeout=120)
    super().Close()
    self._StopBrowser()
    if self._tmp_minidump_dir:
      shutil.rmtree(self._tmp_minidump_dir, ignore_errors=True)
      self._tmp_minidump_dir = None

  def IsBrowserRunning(self):
    return len(self._GetBrowserProcesses()) > 0

  def GetStandardOutput(self):
    return self.platform_backend.GetStandardOutput()

  def PullMinidumps(self):
    self._PullMinidumpsAndAdjustMtimes()

  def CollectDebugData(self, log_level):
    """Collects various information that may be useful for debugging.

    In addition to any data collected by parents' implementation, this also
    collects the following and stores it as artifacts:
      1. UI state of the device
      2. Logcat
      3. Symbolized logcat
      4. Tombstones

    Args:
      log_level: The logging level to use from the logging module, e.g.
          logging.ERROR.

    Returns:
      A debug_data.DebugData object containing the collected data.
    """
    # Store additional debug information as artifacts.
    # We do these in a mixed order so that higher priority ones are done first.
    # This is so that if an error occurs during debug data collection (e.g.
    # adb issues), we're more likely to end up with useful debug information.
    suffix = artifact_logger.GetTimestampSuffix()
    self._StoreLogcatAsArtifact(suffix)
    retval = super().CollectDebugData(log_level)
    self._StoreUiDumpAsArtifact(suffix)
    self._StoreTombstonesAsArtifact(suffix)
    return retval

  def SymbolizeMinidump(self, minidump_path):
    dump_symbolizer = android_minidump_symbolizer.AndroidMinidumpSymbolizer(
        self._dump_finder, self.build_dir,
        symbols_dir=self._CreateExecutableUniqueDirectory('chrome_symbols_'))
    stack = dump_symbolizer.SymbolizeMinidump(minidump_path)
    if not stack:
      return (False, 'Failed to symbolize minidump.')
    self._symbolized_minidump_paths.add(minidump_path)
    return (True, stack)

  def _GetBrowserExecutablePath(self):
    return self._local_apk_path

  def _PullMinidumpsAndAdjustMtimes(self):
    """Pulls any minidumps from the device to the host.

    Skips pulling any dumps that have already been pulled. The modification time
    of any pulled dumps will be set to the modification time of the dump on the
    device, offset by any difference in clocks between the device and host.
    """
    # The offset is (device_time - host_time), so a positive value means that
    # the device clock is ahead.
    time_offset = self.platform_backend.GetDeviceHostClockOffset()
    device = self.platform_backend.device

    device_dump_path = posixpath.join(
        self.platform_backend.GetDumpLocation(self.browser_package),
        'Crashpad', 'pending')
    if not device.PathExists(device_dump_path):
      logging.warning(
          'Device minidump path %s does not exist - not attempting to pull '
          'minidumps', device_dump_path)
      return
    device_dumps = device.ListDirectory(device_dump_path)
    for dump_filename in device_dumps:
      # Skip any .lock files since they're not useful and are prone to being
      # deleted by the time we try to actually pull them.
      if dump_filename.endswith('.lock'):
        continue
      host_path = os.path.join(self._tmp_minidump_dir, dump_filename)
      if os.path.exists(host_path):
        continue
      device_path = posixpath.join(device_dump_path, dump_filename)
      # Skip any files that have a .lock file associated with them, as that
      # implies that the minidump hasn't been fully written to disk yet.
      device_lock_path = device_path + '.lock'
      if device.FileExists(device_lock_path):
        logging.debug('Not pulling file %s because a .lock file exists for it',
                      device_path)
        continue
      device.PullFile(device_path, host_path)
      # Set the local version's modification time to the device's
      # The mtime returned by device_utils.StatPath only has a resolution down
      # to the minute, so we can't use that.
      # On Android L and earlier, 'stat' is not available, so fall back to
      # device_utils.StatPath and adjust the returned value so that it's as new
      # as possible while still fitting in that 1 minute resolution.
      device_mtime = None
      if device.build_version_sdk >= version_codes.MARSHMALLOW:
        device_mtime = device.RunShellCommand(
            ['stat', '-c', '%Y', device_path], single_line=True)
        device_mtime = int(device_mtime.strip())
      else:
        stat_output = device.StatPath(device_path)
        device_mtime = stat_output['st_mtime'] + 59
      host_mtime = device_mtime - time_offset
      os.utime(host_path, (host_mtime, host_mtime))

  def _StoreUiDumpAsArtifact(self, suffix):
    try:
      ui_dump = self.platform_backend.GetSystemUi().ScreenDump()
      artifact_name = posixpath.join(
          self.DEBUG_ARTIFACT_PREFIX, 'ui_dump-%s.txt' % suffix)
      artifact_logger.CreateArtifact(artifact_name, '\n'.join(ui_dump))
    except Exception:  # pylint: disable=broad-except
      logging.exception('Failed to store UI dump')

  def _StoreLogcatAsArtifact(self, suffix):
    # Get more lines than the default since this is likely to be run after a
    # failure, so this gives us a better chance of getting useful debug info.
    logcat = self.platform_backend.GetLogCat(number_of_lines=3000)
    artifact_name = posixpath.join(
        self.DEBUG_ARTIFACT_PREFIX, 'logcat-%s.txt' % suffix)
    artifact_logger.CreateArtifact(artifact_name, logcat)

    symbolized_logcat = self.platform_backend.SymbolizeLogCat(logcat)
    if symbolized_logcat is None:
      symbolized_logcat = 'Failed to symbolize logcat. Is the script available?'
    artifact_name = posixpath.join(
        self.DEBUG_ARTIFACT_PREFIX, 'symbolized_logcat-%s.txt' % suffix)
    artifact_logger.CreateArtifact(artifact_name, symbolized_logcat)

  def _StoreTombstonesAsArtifact(self, suffix):
    tombstones = self.platform_backend.GetTombstones()
    if tombstones is None:
      tombstones = 'Failed to get tombstones. Is the script available?'
    artifact_name = posixpath.join(
        self.DEBUG_ARTIFACT_PREFIX, 'tombstones-%s.txt' % suffix)
    artifact_logger.CreateArtifact(artifact_name, tombstones)
