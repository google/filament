# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import os
import posixpath
import re
import subprocess
import threading
import time
import six

from telemetry.core import android_platform
from telemetry.core import exceptions
from telemetry.core import util
from telemetry import compat_mode_options
from telemetry import decorators
from telemetry.internal.forwarders import android_forwarder
from telemetry.internal.platform import android_device
from telemetry.internal.platform import linux_based_platform_backend
from telemetry.internal.util import binary_manager
from telemetry.internal.util import external_modules
from telemetry.testing import test_utils

from devil.android import app_ui
from devil.android import battery_utils
from devil.android import cpu_temperature
from devil.android import device_errors
from devil.android import device_utils
from devil.android.perf import cache_control
from devil.android.perf import perf_control
from devil.android.perf import thermal_throttle
from devil.android.sdk import shared_prefs
from devil.android.tools import provision_devices
from devil.android.tools import system_app
from devil.android.tools import video_recorder

try:
  # devil.android.forwarder uses fcntl, which doesn't exist on Windows.
  from devil.android import forwarder
except ImportError:
  forwarder = None

try:
  from devil.android.perf import surface_stats_collector
except Exception: # pylint: disable=broad-except
  surface_stats_collector = None

psutil = external_modules.ImportOptionalModule('psutil')

_ARCH_TO_STACK_TOOL_ARCH = {
    'armeabi-v7a': 'arm',
    'arm64-v8a': 'arm64',
}
_MAP_TO_USER_FRIENDLY_OS_NAMES = {
    'k': 'kitkat',
    'l': 'lollipop',
    'm': 'marshmallow',
    'n': 'nougat',
    'o': 'oreo',
    'p': 'pie',
}
_MAP_TO_USER_FRIENDLY_DEVICE_NAMES = {
    'gobo': 'go',
    'W6210': 'one',
    'AOSP on Shamu': 'nexus 6',
    'AOSP on BullHead': 'nexus 5x',
    'wembley_2GB': 'wembley'
}
_NON_ROOT_OVERRIDES = {
    # The Samsung A23 uses the legacy sdcardfs which allows us to read/remove
    # the profile directory directly without root. This allows the device to
    # use the same code path as a rooted device as long as a particular public
    # directory is used. See crbug.com/1383609 for more background on this.
    'SM-A235M': {
        'profile_dir': '/sdcard/Android/data',
        'clear_application_state': False,
    },
}
_DEVICE_COPY_SCRIPT_FILE = os.path.abspath(os.path.join(
    os.path.dirname(__file__), 'efficient_android_directory_copy.sh'))
_DEVICE_COPY_SCRIPT_LOCATION = (
    '/data/local/tmp/efficient_android_directory_copy.sh')
_DEVICE_MEMTRACK_HELPER_LOCATION = '/data/local/tmp/profilers/memtrack_helper'
_DEVICE_CLEAR_SYSTEM_CACHE_TOOL_LOCATION = '/data/local/tmp/clear_system_cache'


class _VideoRecorder():
  def __init__(self):
    self._stop_recording_signal = threading.Event()
    self._recording_path = None
    self._runner = None

  def WaitForSignal(self):
    self._stop_recording_signal.wait()

  @property
  def recording_path(self):
    return self._recording_path

  def Start(self, device):
    def record_video(device, state):
      recorder = video_recorder.VideoRecorder(device)
      with recorder:
        state.WaitForSignal()
      if state.recording_path:
        f = recorder.Pull(state.recording_path)
        logging.info('Video written to %s' % os.path.abspath(f))

    # Start recording the video in parallel to running the story, so that the
    # video recording here does not block running the story (which involve
    # executing additional commands in parallel on the device).
    parallel_devices = device_utils.DeviceUtils.parallel([device], asyn=True)
    self._runner = parallel_devices.pMap(record_video, self)

  def Stop(self, video_path):
    self._recording_path = video_path
    self._stop_recording_signal.set()
    # Recording the video may take a few seconds in the extreme cases. So allow
    # a few seconds when shutting down the recording.
    self._runner.pGet(timeout=10)


class AndroidPlatformBackend(
    linux_based_platform_backend.LinuxBasedPlatformBackend):
  def __init__(self, device, require_root):
    assert device, (
        'AndroidPlatformBackend can only be initialized from remote device')
    super().__init__(device)
    self._device = device_utils.DeviceUtils(device.device_id)
    # Disable Play Store on Android devices.
    if self._device.IsApplicationInstalled('com.android.vending'):
      self._device.RunShellCommand(
        ['pm', 'disable-user', 'com.android.vending'],
        check_return=True)
    self._can_elevate_privilege = False
    self._require_root = require_root
    if self._require_root:
      # Trying to root the device, if possible.
      if not self._device.HasRoot():
        try:
          self._device.EnableRoot()
        except device_errors.CommandFailedError:
          logging.warning('Unable to root %s', str(self._device))
      self._can_elevate_privilege = (
          self._device.HasRoot() or self._device.NeedsSU())
      assert self._can_elevate_privilege, (
          'Android device must have root access to run Telemetry')
    self._battery = battery_utils.BatteryUtils(self._device)
    self._surface_stats_collector = None
    self._perf_tests_setup = perf_control.PerfControl(self._device)
    self._thermal_throttle = thermal_throttle.ThermalThrottle(self._device)
    self._raw_display_frame_rate_measurements = []
    self._device_copy_script = None
    self._system_ui = None
    self._device_host_clock_offset = None
    self._video_recorder = None

    # TODO(https://crbug.com/1026296): Remove this once --chromium-output-dir
    # has a default value we can use.
    self._build_dir = util.GetUsedBuildDirectory()

    _FixPossibleAdbInstability()

  @property
  def log_file_path(self):
    return None

  @classmethod
  def SupportsDevice(cls, device):
    return isinstance(device, android_device.AndroidDevice)

  @classmethod
  def CreatePlatformForDevice(cls, device, finder_options):
    assert cls.SupportsDevice(device)
    require_root = (compat_mode_options.DONT_REQUIRE_ROOTED_DEVICE not in
                    finder_options.browser_options.compatibility_mode)
    platform_backend = AndroidPlatformBackend(device, require_root)
    return android_platform.AndroidPlatform(platform_backend)

  def _CreateForwarderFactory(self):
    return android_forwarder.AndroidForwarderFactory(self._device)

  @property
  def device(self):
    return self._device

  @property
  def require_root(self):
    return self._require_root

  def Initialize(self):
    self.EnsureBackgroundApkInstalled()

  def GetSystemUi(self):
    if self._system_ui is None:
      self._system_ui = app_ui.AppUi(self.device, 'com.android.systemui')
    return self._system_ui

  def GetSharedPrefs(self, package, filename, use_encrypted_path=False):
    """Creates a Devil SharedPrefs instance.

    See devil.android.sdk.shared_prefs for the documentation of the returned
    object.

    Args:
      package: A string containing the package of the app that the SharedPrefs
          instance will be for.
      filename: A string containing the specific settings file of the app that
          the SharedPrefs instance will be for.
      use_encrypted_path: Whether to use the newer device-encrypted path
          (/data/user_de/) instead of the older unencrypted path (/data/data/).

    Returns:
      A reference to a SharedPrefs object for the given package and filename
      on whatever device the platform backend has a reference to.
    """
    return shared_prefs.SharedPrefs(
        self._device, package, filename, use_encrypted_path=use_encrypted_path)

  def IsSvelte(self):
    # TODO (crbug.com/973936) This function is used to find out if expectations
    # with the Android_Svelte tag should apply to a certain build in the old
    # expectations format. Instead of eliminating the Android_Svelte tag in the
    # old format, we should wait until we change the expectations into the new
    # format. And then we can get rid of this function.
    description = self._device.GetProp('ro.build.description', cache=True)
    return description and 'svelte' in description

  def IsLowEnd(self):
    return self.IsSvelte() or self.GetDeviceTypeName() in ('gobo', 'W6210')

  def IsAosp(self):
    description = self._device.GetProp('ro.build.description', cache=True)
    if description is not None:
      return 'aosp' in description
    return False


  def GetRemotePort(self, port):
    return forwarder.Forwarder.DevicePortForHostPort(port) or 0

  def IsRemoteDevice(self):
    # Android device is connected via adb which is on remote.
    return True

  def IsDisplayTracingSupported(self):
    return bool(self.GetOSVersionName() >= 'J')

  def StartDisplayTracing(self):
    assert not self._surface_stats_collector
    # Clear any leftover data from previous timed out tests
    self._raw_display_frame_rate_measurements = []
    self._surface_stats_collector = \
        surface_stats_collector.SurfaceStatsCollector(self._device)
    self._surface_stats_collector.Start()

  def StopDisplayTracing(self):
    if not self._surface_stats_collector:
      return None

    try:
      refresh_period, timestamps = self._surface_stats_collector.Stop()
      pid = self._surface_stats_collector.GetSurfaceFlingerPid()
    finally:
      self._surface_stats_collector = None
    # TODO(sullivan): should this code be inline, or live elsewhere?
    events = [_BuildEvent(
        '__metadata', 'process_name', 'M', pid, 0, {'name': 'SurfaceFlinger'})]
    for ts in timestamps:
      events.append(_BuildEvent('SurfaceFlinger', 'vsync_before', 'I', pid, ts,
                                {'data': {'frame_count': 1}}))

    return {
        'traceEvents': events,
        'metadata': {
            'clock-domain': 'LINUX_CLOCK_MONOTONIC',
            'surface_flinger': {
                'refresh_period': refresh_period,
            },
        }
    }

  def CanTakeScreenshot(self):
    return True

  def TakeScreenshot(self, file_path):
    return bool(self._device.TakeScreenshot(host_path=file_path))

  def CanRecordVideo(self):
    return True

  def StartVideoRecording(self):
    assert self._video_recorder is None
    self._video_recorder = _VideoRecorder()
    self._video_recorder.Start(self.device)

  def StopVideoRecording(self, video_path):
    assert self._video_recorder
    self._video_recorder.Stop(video_path)
    self._video_recorder = None

  def CooperativelyShutdown(self, proc, app_name):
    # Suppress the 'abstract-method' lint warning.
    return False

  def SetPerformanceMode(self, performance_mode):
    if not self._can_elevate_privilege:
      logging.warning('No root privileges, so ignoring performance mode.')
      return
    if performance_mode == android_device.KEEP_PERFORMANCE_MODE:
      logging.info('Keeping device performance settings intact.')
      return
    if performance_mode == android_device.HIGH_PERFORMANCE_MODE:
      logging.info('Setting high performance mode.')
      self._perf_tests_setup.SetHighPerfMode()
    elif performance_mode == android_device.NORMAL_PERFORMANCE_MODE:
      logging.info('Setting normal performance mode.')
      self._perf_tests_setup.SetDefaultPerfMode()
    elif performance_mode == android_device.LITTLE_ONLY_PERFORMANCE_MODE:
      logging.info('Setting little-only performance mode.')
      self._perf_tests_setup.SetLittleOnlyMode()
    else:
      raise ValueError('Unknown performance mode: %s' % performance_mode)


  def CanMonitorThermalThrottling(self):
    return True

  def IsThermallyThrottled(self):
    return self._thermal_throttle.IsThrottled()

  def HasBeenThermallyThrottled(self):
    return self._thermal_throttle.HasBeenThrottled()

  def SetGraphicsMemoryTrackingEnabled(self, enabled):
    if not enabled:
      self.KillApplication('memtrack_helper')
      return

    binary_manager.ReinstallAndroidHelperIfNeeded(
        'memtrack_helper', _DEVICE_MEMTRACK_HELPER_LOCATION,
        self._device)
    self._device.RunShellCommand(
        [_DEVICE_MEMTRACK_HELPER_LOCATION, '-d'], as_root=True,
        check_return=True)

  def EnsureBackgroundApkInstalled(self):
    app = 'push_apps_to_background_apk'
    arch_name = self._device.GetABI()
    host_path = binary_manager.FetchPath(app, 'android', arch_name)
    if not host_path:
      raise Exception('Error installing PushAppsToBackground.apk.')
    self.InstallApplication(host_path)

  @decorators.Cache
  def GetArchName(self):
    return self._device.GetABI()

  def GetOSName(self):
    return 'android'

  def GetDeviceId(self):
    return self._device.serial

  def GetDeviceTypeName(self):
    return self._device.product_model

  def GetTypExpectationsTags(self):
    os_release_version = self.GetOSReleaseVersion()
    tags = [
        self.GetOSName(),
        f'android-{os_release_version}',
    ]
    # Starting in 2024, the Android image naming scheme changed so that the
    # first letter no longer corresponds to the codename, e.g. Android 14 is
    # no longer Android U. Instead, the release version should be used directly.
    # The letter version is kept around for backwards compatibility for OS
    # versions that stopped being updated prior to the naming change. See
    # crbug.com/333795261 for details.
    if int(os_release_version) <= 13:
      os_version = self.GetOSVersionName().lower()
      os_version = _MAP_TO_USER_FRIENDLY_OS_NAMES.get(os_version, os_version)
      tags.append(f'android-{os_version}')
    tags = test_utils.sanitizeTypExpectationsTags(tags)

    # telemetry benchmark's expectations need to know the model name
    # and if it is a low end device
    device_type_name = self.GetDeviceTypeName()
    logging.info('Android device type name: %s', device_type_name)
    tags += test_utils.sanitizeTypExpectationsTags(
        ['android-' + _MAP_TO_USER_FRIENDLY_DEVICE_NAMES.get(
            device_type_name, device_type_name)])
    if self.IsLowEnd():
      tags.append('android-low-end')
    tags.append('mobile')
    return tags

  @decorators.Cache
  def GetOSVersionName(self):
    return self._device.GetProp('ro.build.id')[0]

  def GetOSVersionDetailString(self):
    return self._device.GetProp('ro.build.id')

  @decorators.Cache
  def GetOSReleaseVersion(self):
    """Gets the numerical Android release version.

    Any minor or patch versions are stripped off.
    """
    return self._device.GetProp('ro.build.version.release').split('.')[0]

  def GetDeviceHostClockOffset(self):
    """Returns the difference between the device and host clocks."""
    if self._device_host_clock_offset is None:
      # Get the current time in seconds since the epoch.
      device_time = self.device.RunShellCommand(
          ['date', '+%s'], single_line=True)
      host_time = time.time()
      self._device_host_clock_offset = int(int(device_time.strip()) - host_time)
    return self._device_host_clock_offset

  def CanFlushIndividualFilesFromSystemCache(self):
    return True

  def SupportFlushEntireSystemCache(self):
    return self._can_elevate_privilege

  def FlushEntireSystemCache(self):
    cache = cache_control.CacheControl(self._device)
    cache.DropRamCaches()

  def FlushSystemCacheForDirectory(self, directory):
    binary_manager.ReinstallAndroidHelperIfNeeded(
        'clear_system_cache', _DEVICE_CLEAR_SYSTEM_CACHE_TOOL_LOCATION,
        self._device)
    self._device.RunShellCommand(
        [_DEVICE_CLEAR_SYSTEM_CACHE_TOOL_LOCATION, '--recurse', directory],
        as_root=True, check_return=True)

  def FlushDnsCache(self):
    self._device.RunShellCommand(
        ['ndc', 'resolver', 'flushdefaultif'],
        as_root=True, check_return=self._require_root)

  def StopApplication(self, application):
    """Stop the given |application|.

    Args:
       application: The full package name string of the application to stop.
    """
    self._device.ForceStop(application)

  def KillApplication(self, application):
    """Kill the given |application|.

    Might be used instead of ForceStop for efficiency reasons.

    Args:
      application: The full package name string of the application to kill.
    """
    assert isinstance(application, six.string_types)
    self._device.KillAll(application, blocking=True, quiet=True, as_root=True)

  def LaunchApplication(
      self, application, parameters=None, elevate_privilege=False):
    """Launches the given |application| with a list of |parameters| on the OS.

    Args:
      application: The full package name string of the application to launch.
      parameters: A list of parameters to be passed to the ActivityManager.
      elevate_privilege: Currently unimplemented on Android.
    """
    if elevate_privilege:
      raise NotImplementedError("elevate_privilege isn't supported on android.")
    # TODO(catapult:#3215): Migrate to StartActivity.
    cmd = ['am', 'start']
    if parameters:
      cmd.extend(parameters)
    cmd.append(application)
    result_lines = self._device.RunShellCommand(cmd, check_return=True)
    for line in result_lines:
      if line.startswith('Error: '):
        raise ValueError('Failed to start "%s" with error\n  %s' %
                         (application, line))

  def StartActivity(self, intent, blocking):
    """Starts an activity for the given intent on the device."""
    self._device.StartActivity(intent, blocking=blocking)

  def CanLaunchApplication(self, application):
    return bool(self._device.GetApplicationPaths(application))

  # pylint: disable=arguments-differ
  def InstallApplication(self, application, modules=None):
    self._device.Install(application, modules=modules)

  def RemoveSystemPackages(self, packages):
    system_app.RemoveSystemApps(self._device, packages)

  def PathExists(self, device_path, **kwargs):
    """ Return whether the given path exists on the device.
    This method is the same as
    devil.android.device_utils.DeviceUtils.PathExists.
    """
    return self._device.PathExists(device_path, **kwargs)

  def GetFileContents(self, fname):
    if not self._can_elevate_privilege:
      logging.warning('%s cannot be retrieved on non-rooted device.', fname)
      return ''
    return self._device.ReadFile(fname, as_root=True)

  def RunCommand(self, command):
    return '\n'.join(self._device.RunShellCommand(
        command,
        shell=isinstance(command, six.string_types), check_return=True))

  def SetRelaxSslCheck(self, value):
    old_flag = self._device.GetProp('socket.relaxsslcheck')
    self._device.SetProp('socket.relaxsslcheck', value)
    return old_flag

  def DismissCrashDialogIfNeeded(self):
    """Dismiss any error dialogs.

    Limit the number in case we have an error loop or we are failing to dismiss.
    """
    for _ in range(10):
      if not self._device.DismissCrashDialogIfNeeded():
        break

  def PushProfile(self, package, new_profile_dir):
    """Replace application profile with files found on host machine.

    Pushing the profile is slow, so we don't want to do it every time.
    Avoid this by pushing to a safe location using PushChangedFiles, and
    then copying into the correct location on each test run.

    Args:
      package: The full package name string of the application for which the
        profile is to be updated.
      new_profile_dir: Location where profile to be pushed is stored on the
        host machine.
    """
    (profile_parent, profile_base) = os.path.split(new_profile_dir)
    # If the path ends with a '/' python split will return an empty string for
    # the base name; so we now need to get the base name from the directory.
    if not profile_base:
      profile_base = os.path.basename(profile_parent)

    provision_devices.CheckExternalStorage(self._device)

    saved_profile_location = posixpath.join(
        self._device.GetExternalStoragePath(),
        'profile', profile_base)
    self._device.PushChangedFiles([(new_profile_dir, saved_profile_location)],
                                  delete_device_stale=True)

    profile_dir = self.GetProfileDir(package)
    self._EfficientDeviceDirectoryCopy(
        saved_profile_location, profile_dir)
    dumpsys = self._device.RunShellCommand(
        ['dumpsys', 'package', package], check_return=True)
    id_line = next(line for line in dumpsys if 'userId=' in line)
    uid = re.search(r'\d+', id_line).group()

    # Generate all of the paths copied to the device, via walking through
    # |new_profile_dir| and doing path manipulations. This could be replaced
    # with recursive commands (e.g. chown -R) below, but those are not well
    # supported by older Android versions.
    device_paths = []
    for root, dirs, files in os.walk(new_profile_dir):
      rel_root = os.path.relpath(root, new_profile_dir)
      posix_rel_root = rel_root.replace(os.sep, posixpath.sep)

      device_root = posixpath.normpath(posixpath.join(profile_dir,
                                                      posix_rel_root))

      if rel_root == '.' and 'lib' in files:
        files.remove('lib')
      device_paths.extend(posixpath.join(device_root, n) for n in files + dirs)

    owner_group = '%s.%s' % (uid, uid)
    self._device.ChangeOwner(owner_group, device_paths)

    # Not having the correct SELinux security context can prevent Chrome from
    # loading files even though the mode/group/owner combination should allow
    # it.
    security_context = self._device.GetSecurityContextForPackage(package)
    self._device.ChangeSecurityContext(security_context, device_paths)

  def _EfficientDeviceDirectoryCopy(self, source, dest):
    if not self._device_copy_script:
      self._device.adb.Push(
          _DEVICE_COPY_SCRIPT_FILE,
          _DEVICE_COPY_SCRIPT_LOCATION)
      self._device_copy_script = _DEVICE_COPY_SCRIPT_LOCATION
    self._device.RunShellCommand(
        ['sh', self._device_copy_script, source, dest], check_return=True)

  def RemoveProfile(self, package, ignore_list, permissions=None):
    """Delete application profile on device.

    Args:
      package: The full package name string of the application for which the
        profile is to be deleted.
      ignore_list: List of files to keep.
      permissions: An optional list of strings containing the permissions to
        grant after removing the profile. Only relevant for non-rooted devices.
    """
    # If we don't have root, then we are almost certainly actually using the
    # default profile directory instead of the one we return from GetProfileDir.
    # This current best guess for this behavior is that it is due to SELinux,
    # as a non-root-readable directory will not be usable by the browser due to
    # SELinux security contexts/permissions. So, clear the default directory by
    # wiping the application state. We still go through the regular profile
    # directory deletion afterwards on the off chance that we are somehow using
    # the non-default directory.
    if not self._require_root and _NON_ROOT_OVERRIDES.get(
        self.GetDeviceTypeName(), {}).get('clear_application_state', True):
      if permissions is None:
        logging.warning('Clearing application state to remove profile on a '
                        'non-rooted device, but no permissions were provided. '
                        'Permissions will likely not be set properly.')
      # We specify to wait for the asynchronous intent since there have been
      # known problems with it deleting data out from under a test. See
      # crbug.com/1383609 for an example.
      self._device.ClearApplicationState(package,
                                         permissions=permissions,
                                         wait_for_asynchronous_intent=True)
    profile_dir = self.GetProfileDir(package)
    if not self._device.PathExists(profile_dir):
      return
    files = [
        posixpath.join(profile_dir, f)
        for f in self._device.ListDirectory(profile_dir, as_root=True)
        if f not in ignore_list]
    if not files:
      return
    self._device.RemovePath(files, force=True, recursive=True, as_root=True)

  def GetProfileDir(self, package):
    """Returns the on-device location where the application profile is stored
    based on Android convention.

    Args:
      package: The full package name string of the application.
    """
    if self._require_root:
      return '/data/data/%s/' % package
    # We use a public location to ensure minidumps can be pulled without root.
    # /data/local/tmp/package/ seems like it would be more fitting, but for
    # some reason (maybe related to SELinux), using that directory does not
    # work. If the package directory doesn't exist, then Chromium ends up
    # defaulting to /data/data/package/ instead. If the directory does exist,
    # Chromium ends up segfaulting somewhere.
    profile_dir = _NON_ROOT_OVERRIDES.get(
        self.GetDeviceTypeName(), {}).get('profile_dir', '/sdcard/Download')
    return posixpath.join(profile_dir, package) + '/'

  def GetDumpLocation(self, package):
    """Returns the location where crash dumps should be written to.

    Args:
      package: A string containing the package name of the application that the
          dump location is for.
    """
    # On Android Q+, apps by default only have access to certain directories,
    # so use a subdirectory of the package's profile directory to guarantee that
    # it has access to it.
    return self.GetProfileDir(package) + 'dumps'

  def SetDebugApp(self, package):
    """Set application to debugging.

    Args:
      package: The full package name string of the application.
    """
    if self._device.IsUserBuild():
      logging.debug('User build device, setting debug app')
      self._device.RunShellCommand(
          ['am', 'set-debug-app', '--persistent', package],
          check_return=True)

  def GetLogCat(self, number_of_lines=1500):
    """Returns most recent lines of logcat dump.

    Args:
      number_of_lines: Number of lines of log to return.
    """
    def decode_line(line):
      # Both input and output are of 'str' type, in both Python 2 and 3.
      # However, note that in Python 2 str is a series of bytes,
      # while in Python 3 it is Unicode string.
      try:
        if six.PY2:
          # str -> unicode
          uline = six.text_type(line, encoding='utf-8')
          # unicode -> str (ASCII with special characters encoded)
          return uline.encode('ascii', 'backslashreplace')
        # str -> bytes (ASCII with special characters encoded)
        bline = line.encode('ascii', 'backslashreplace')
        # bytes -> str
        return bline.decode('ascii')
      except Exception: # pylint: disable=broad-except
        logging.error('Error encoding UTF-8 logcat line as ASCII.')
        return '<MISSING LOGCAT LINE: FAILED TO ENCODE>'

    logcat_output = self._device.RunShellCommand(
        ['logcat', '-d', '-t', str(number_of_lines)],
        check_return=True, large_output=True)
    return '\n'.join(decode_line(l) for l in logcat_output)

  def SymbolizeLogCat(self, logcat):
    """Attempts to symbolize any crash stacks in the given logcat data.

    Args:
      logcat: A string containing the logcat data to be symbolized.

    Returns:
      A string containing the symbolized logcat data, or None if the symbolize
      script was not found.
    """
    stack = os.path.join(util.GetChromiumSrcDir(), 'third_party',
                         'android_platform', 'development', 'scripts', 'stack')
    if _ExecutableExists(stack):
      cmd = [stack]
      arch = self.GetArchName()
      arch = _ARCH_TO_STACK_TOOL_ARCH.get(arch, arch)
      cmd.append('--arch=%s' % arch)
      cmd.append('--output-directory=%s' % self._build_dir)
      p = subprocess.Popen(
          cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
      return p.communicate(input=six.ensure_text(logcat))[0]

    return None

  def GetTombstones(self):
    """Attempts to get any tombstones currently on the device.

    Returns:
      A string containing any tombstones found on the device, or None if the
      tombstones script was not found or failed.
    """
    tombstones = os.path.join(util.GetChromiumSrcDir(), 'build', 'android',
                              'tombstones.py')
    if _ExecutableExists(tombstones):
      tombstones_cmd = [
          tombstones, '-w',
          '--device', self._device.adb.GetDeviceSerial(),
          '--adb-path', self._device.adb.GetAdbPath(),
          '--output-directory=%s' % self._build_dir,
      ]
      try:
        return subprocess.check_output(tombstones_cmd)
      except subprocess.CalledProcessError:
        return None

    return None

  def GetStandardOutput(self):
    return 'Cannot get standard output on Android'

  def IsScreenOn(self):
    """Determines if device screen is on."""
    return self._device.IsScreenOn()

  @staticmethod
  def _ExtractLastNativeCrashPackageFromLogcat(
      logcat, default_package_name='com.google.android.apps.chrome'):
    # pylint: disable=line-too-long
    # Match against lines like:
    # <unimportant prefix> : Fatal signal 5 (SIGTRAP), code -6 in tid NNNNN (oid.apps.chrome)
    # <a few more lines>
    # <unimportant prefix>: Build fingerprint: 'google/bullhead/bullhead:7.1.2/N2G47F/3769476:userdebug/dev-keys'
    # <a few more lines>
    # <unimportant prefix> : pid: NNNNN, tid: NNNNN, name: oid.apps.chrome  >>> com.google.android.apps.chrome <<<
    # pylint: enable=line-too-long
    fatal_signal_re = re.compile(r'.*: Fatal signal [0-9]')
    build_fingerprint_re = re.compile(r'.*: Build fingerprint: ')
    package_re = re.compile(r'.*: pid: [0-9]+, tid: [0-9]+, name: .*'
                            r'>>> (?P<package_name>[^ ]+) <<<')
    last_package = default_package_name
    build_fingerprint_found = False
    lookahead_lines_remaining = 0
    for line in logcat.splitlines():
      if fatal_signal_re.match(line):
        lookahead_lines_remaining = 10
        continue
      if not lookahead_lines_remaining:
        build_fingerprint_found = False
      else:
        lookahead_lines_remaining -= 1
        if build_fingerprint_re.match(line):
          build_fingerprint_found = True
          continue
        if build_fingerprint_found:
          m = package_re.match(line)
          if m:
            last_package = m.group('package_name')
            # The package name may have a trailing process name in it,
            # for example: "org.chromium.chrome:privileged_process0".
            last_package = last_package.split(':')[0]
    return last_package

  @staticmethod
  def _IsScreenLocked(input_methods):
    """Parser method for IsScreenLocked()

    Args:
      input_methods: Output from dumpsys input_methods

    Returns:
      boolean: True if screen is locked, false if screen is not locked.

    Raises:
      ValueError: An unknown value is found for the screen lock state.
      AndroidDeviceParsingError: Error in detecting screen state.

    """
    for line in input_methods:
      if 'mHasBeenInactive' in line:
        for pair in line.strip().split(' '):
          key, value = pair.split('=', 1)
          if key == 'mHasBeenInactive':
            if value == 'true':
              return True
            if value == 'false':
              return False
            raise ValueError('Unknown value for %s: %s' % (key, value))
    raise exceptions.AndroidDeviceParsingError(str(input_methods))

  def IsScreenLocked(self):
    """Determines if device screen is locked."""
    input_methods = self._device.RunShellCommand(['dumpsys', 'input_method'],
                                                 check_return=True)
    return self._IsScreenLocked(input_methods)

  def Log(self, message):
    """Prints line to logcat."""
    TELEMETRY_LOGCAT_TAG = 'Telemetry'
    self._device.RunShellCommand(
        ['log', '-p', 'i', '-t', TELEMETRY_LOGCAT_TAG, message],
        check_return=True)

  def WaitForBatteryTemperature(self, temp):
    # Temperature is in tenths of a degree C, so we convert to that scale.
    self._battery.LetBatteryCoolToTemperature(temp * 10)

  def WaitForCpuTemperature(self, temp):
    controller = cpu_temperature.CpuTemperature(self._device)
    # Check if the device temperature zones are known
    if controller.IsSupported():
      controller.LetCpuCoolToTemperature(temp)
    else:
      logging.warning('CPU temperature cooling delay - '
                      'CPU temperature cannot be read: Either the current '
                      'device is not supported or the specified temperature '
                      'zones do not exist.')


def _FixPossibleAdbInstability():
  """Host side workaround for crbug.com/268450 (adb instability).

  The adb server has a race which is mitigated by binding to a single core.
  """
  if not psutil:
    return
  for process in psutil.process_iter():
    try:
      if psutil.version_info >= (2, 0):
        if 'adb' in process.name():
          process.cpu_affinity([0])
      else:
        if 'adb' in process.name:
          process.set_cpu_affinity([0])
    except (psutil.NoSuchProcess, psutil.AccessDenied):
      logging.warning('Failed to set adb process CPU affinity')


def _BuildEvent(cat, name, ph, pid, ts, args):
  event = {
      'cat': cat,
      'name': name,
      'ph': ph,
      'pid': pid,
      'tid': pid,
      'ts': ts * 1000,
      'args': args
  }
  # Instant events need to specify the scope, too.
  if ph == 'I':
    event['s'] = 't'
  return event


def _ExecutableExists(file_name):
  return os.access(file_name, os.X_OK)
