# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import contextlib
import logging
import os
import shutil
import subprocess
import six.moves.urllib.parse # pylint: disable=import-error

from telemetry.internal.util import binary_manager
from telemetry.util import statistics

from devil.android.sdk import version_codes

__all__ = ['BrowserIntervalProfilingController']

class BrowserIntervalProfilingController():
  def __init__(self, possible_browser, process_name, periods, frequency,
               profiler_options):
    process_name, _, thread_name = process_name.partition(':')
    self._periods = periods
    self._frequency = statistics.Clamp(int(frequency), 1, 4000)
    self._platform_controller = None
    if periods and self._IsIntervalProfilingSupported(possible_browser):
      self._platform_controller = self._CreatePlatformController(
          possible_browser, process_name, thread_name, profiler_options)

  @staticmethod
  def _IsIntervalProfilingSupported(possible_browser):
    os_name = possible_browser._platform_backend.GetOSName()
    is_supported_android = os_name == 'android' and (
        possible_browser._platform_backend.device.build_version_sdk >=
        version_codes.NOUGAT)
    return os_name == 'linux' or os_name == 'chromeos' or is_supported_android

  @staticmethod
  def _CreatePlatformController(possible_browser, process_name, thread_name,
                                profiler_options):
    os_name = possible_browser._platform_backend.GetOSName()
    if os_name == 'linux':
      return _LinuxController(possible_browser, process_name, thread_name,
                              profiler_options)
    if os_name == 'android':
      return _AndroidController(possible_browser, process_name, thread_name,
                                profiler_options)
    if os_name == 'chromeos':
      return _ChromeOSController(possible_browser, process_name, thread_name,
                                 profiler_options)
    return None

  @contextlib.contextmanager
  def SamplePeriod(self, period, action_runner):
    if not self._platform_controller or period not in self._periods:
      yield
      return

    with self._platform_controller.SamplePeriod(
        period=period,
        action_runner=action_runner,
        frequency=self._frequency):
      yield

  def GetResults(self, file_safe_name, results):
    if self._platform_controller:
      self._platform_controller.GetResults(file_safe_name, results)


class _PlatformController():
  def SamplePeriod(self, period, action_runner):
    raise NotImplementedError()

  def GetResults(self, file_safe_name, results):
    raise NotImplementedError()


class _LinuxController(_PlatformController):
  PERF_BINARY_PATH = '/usr/bin/perf'
  DEVICE_OUT_FILE_PATTERN = '/tmp/{period}-{pid}-perf.data'

  def __init__(self, possible_browser, process_name, thread_name,
               profiler_options):
    super().__init__()
    if profiler_options:
      raise ValueError(
          'Additional arguments to the profiler is not supported on Linux.')
    if process_name != 'renderer' or thread_name != 'main':
      raise ValueError(
          'Only profiling renderer main thread is supported on Linux.'
          ' Got process name "%s" and thread name "%s".'
          % (process_name, thread_name))
    possible_browser.AddExtraBrowserArg('--no-sandbox')
    self._temp_results = []
    self._perf_command = [self.PERF_BINARY_PATH, 'record'] + profiler_options
    self._process_name = process_name
    self._thread_name = thread_name
    self._device_results = []

  @contextlib.contextmanager
  def SamplePeriod(self, period, action_runner, **_):
    """Collects CPU profiles for the giving period."""
    browser = action_runner.tab.browser

    processes = [p for p in browser._browser_backend.processes
                 if p.name == self._process_name]

    # The tests make 2 renderers, only one of which is the one we want to use
    # for profiling. We always get the heavier one here. If this breaks, change
    # this below.
    if self._process_name == 'renderer':
      processes = [p for p in processes if p.name == 'renderer']
      # Sort by RSS, from smallest to biggest.
      processes.sort(key=lambda p: int(p.rss))

    process = processes[-1]
    outfile = (self.DEVICE_OUT_FILE_PATTERN
          .format(period=period, pid=process.pid).replace(' ', ''))
    tmp = ' '.join(self._perf_command + [
        '-g',
        '-e', 'cpu-clock',
        '-p', process.pid,
        '-o', outfile])
    result = subprocess.Popen(tmp.split())
    try:
      yield
    finally:
      result.poll()
      if result.returncode is not None:
        logging.warning('Profiling process exited prematurely.')
      else:
        self._temp_results.append((period, outfile))
        self._StopProfiling()
        result.wait()

  def GetResults(self, file_safe_name, results):
    for period, temp_file in self._temp_results:
      with results.CaptureArtifact(
          'perf-%s-%s.perf.data' % (file_safe_name, period)) as dest_file:
        shutil.move(temp_file, dest_file)
    self._temp_results = []

  def _StopProfiling(self):
    # Kill the profiling process directly.
    subprocess.call(['killall', '-s', 'INT',
                     self.PERF_BINARY_PATH])


class _AndroidController(_PlatformController):
  DEVICE_PROFILERS_DIR = '/data/local/tmp/profilers'
  DEVICE_OUT_FILE_PATTERN = '/data/local/tmp/{period}-perf.data'

  def __init__(self, possible_browser, process_name, thread_name,
               profiler_options):
    super().__init__()
    if profiler_options:
      raise ValueError(
          'Additional arguments to the profiler is not supported on Android.')
    if process_name == 'system_wide':
      raise ValueError(
          'System-wide profiling is not supported on Android.')
    self._device = possible_browser._platform_backend.device
    self._process_name = process_name
    self._thread_name = thread_name
    self._device_simpleperf_path = self._InstallSimpleperf(possible_browser)
    self._device_results = []

  @classmethod
  def _InstallSimpleperf(cls, possible_browser):
    device = possible_browser._platform_backend.device
    package = possible_browser._backend_settings.package

    # Necessary for profiling
    # https://android-review.googlesource.com/c/platform/system/sepolicy/+/234400
    device.SetProp('security.perf_harden', '0')

    # This is the architecture of the app to be profiled, not of the device.
    package_arch = device.GetPackageArchitecture(package) or 'armeabi-v7a'
    host_path = binary_manager.FetchPath(
        'simpleperf', 'android', package_arch)
    if not host_path:
      raise Exception('Could not get path to simpleperf executable on host.')
    device_path = os.path.join(cls.DEVICE_PROFILERS_DIR,
                               package_arch,
                               'simpleperf')
    device.PushChangedFiles([(host_path, device_path)])
    return device_path

  @staticmethod
  def _ThreadsForProcess(device, pid):
    if device.build_version_sdk >= version_codes.OREO:
      pid_regex = (
          '^[[:graph:]]\\{1,\\}[[:blank:]]\\{1,\\}%s[[:blank:]]\\{1,\\}' % pid)
      ps_cmd = "ps -T -e | grep '%s'" % pid_regex
      ps_output_lines = device.RunShellCommand(
          ps_cmd, shell=True, check_return=True)
    else:
      ps_cmd = ['ps', '-p', pid, '-t']
      ps_output_lines = device.RunShellCommand(ps_cmd, check_return=True)
    result = []
    for l in ps_output_lines:
      fields = l.split()
      if fields[2] == pid:
        continue
      result.append((fields[2], fields[-1]))
    return result

  def _StartSimpleperf(self, browser, out_file, frequency):
    device = browser._platform_backend.device

    processes = [p for p in browser._browser_backend.processes
                 if (browser._browser_backend.GetProcessName(p.name)
                     == self._process_name)]
    if len(processes) != 1:
      raise Exception('Found %d running processes with names matching "%s"' %
                      (len(processes), self._process_name))
    pid = processes[0].pid

    profile_cmd = [self._device_simpleperf_path, 'record',
                   '-g', # Enable call graphs based on dwarf debug frame
                   '-f', str(frequency),
                   '-o', out_file]

    if self._thread_name:
      threads = [t for t in self._ThreadsForProcess(device, str(pid))
                 if (browser._browser_backend.GetThreadType(t[1]) ==
                     self._thread_name)]
      if len(threads) != 1:
        raise Exception('Found %d threads with names matching "%s"' %
                        (len(threads), self._thread_name))
      profile_cmd.extend(['-t', threads[0][0]])
    else:
      profile_cmd.extend(['-p', str(pid)])
    return device.adb.StartShell(profile_cmd)

  @contextlib.contextmanager
  def SamplePeriod(self, period, action_runner, **kwargs):
    profiling_process = None
    out_file = self.DEVICE_OUT_FILE_PATTERN.format(period=period)
    frequency = kwargs.get('frequency', 1000)
    profiling_process = self._StartSimpleperf(
        action_runner.tab.browser,
        out_file,
        frequency)
    yield
    device = action_runner.tab.browser._platform_backend.device
    pidof_lines = device.RunShellCommand(['pidof', 'simpleperf'])
    if not pidof_lines:
      raise Exception('Could not get pid of running simpleperf process.')
    device.RunShellCommand(['kill', '-s', 'SIGINT', pidof_lines[0].strip()])
    profiling_process.wait()
    self._device_results.append((period, out_file))

  def GetResults(self, file_safe_name, results):
    for period, device_file in self._device_results:
      with results.CaptureArtifact(
          'simpleperf-%s-%s.perf.data' % (file_safe_name, period)) as dest_file:
        self._device.PullFile(device_file, dest_file)
    self._device_results = []


class _ChromeOSController(_PlatformController):
  PERF_BINARY_PATH = '/usr/bin/perf'
  DEVICE_OUT_FILE_PATTERN = '/tmp/{period}-perf.data'
  DEVICE_KPTR_FILE = '/proc/sys/kernel/kptr_restrict'

  def __init__(self, possible_browser, process_name, thread_name,
               profiler_options):
    super().__init__()
    if process_name != 'system_wide':
      raise ValueError(
          'Only system-wide profiling is supported on ChromeOS.'
          ' Got process name "%s".' % process_name)
    if thread_name != '':
      raise ValueError(
          'Thread name should be empty for system-wide profiling on ChromeOS.'
          ' Got thread name "%s".' % thread_name)
    if not profiler_options:
      raise ValueError('Profiler options must be provided to run the linux perf'
                       ' tool on ChromeOS.')
    self._platform_backend = possible_browser._platform_backend
    # Default to system-wide profile collection as only system-wide profiling
    # is supported on ChromeOS.
    self._perf_command = [self.PERF_BINARY_PATH] + profiler_options + ['-a']
    self._PrepareHostForProfiling()
    self._ValidatePerfCommand()
    self._device_results = []

  def _PrepareHostForProfiling(self):
    """Updates DEVICE_KPTR_FILE.

    It places a 0 value to make kernel mappings available to the user.
    """
    self._platform_backend.PushContents('0', self.DEVICE_KPTR_FILE)
    data = self._platform_backend.GetFileContents(self.DEVICE_KPTR_FILE)
    if data.strip() != '0':
      logging.warning("DEVICE_KPTR_FILE update failed."
                      " Kernel mappings aren't available to non-root users.")

  def _ValidatePerfCommand(self):
    """Constructs and validates the perf command line.

    Validates the arguments passed in the profiler options, constructs the final
    perf command line and validates the perf command line by running it on the
    device.
    """
    # Validate the arguments passed in the profiler options.
    if self._perf_command[1] != "record" and self._perf_command[1] != "stat":
      raise ValueError(
          'Only the record and stat perf subcommands are allowed.'
          ' Got "%s" perf subcommand.' % self._perf_command[1])

    if '-o' in self._perf_command:
      raise ValueError(
          'Cannot pass the output filename flag in the profiler options.'
          ' Constructed command %s.' % self._perf_command)

    if '--' in self._perf_command:
      raise ValueError(
          'Cannot pass a command to run in the profiler options.'
          ' Constructed command %s.' % self._perf_command)

    # Run and validate the final linux perf command.
    cmd = self._perf_command + ['-o', '/dev/null', '-- touch /dev/null']
    p = self._platform_backend.StartCommand(cmd)
    p.wait()
    if p.returncode != 0:
      raise Exception('Perf command validation failed.'
                      ' Executed command "%s" and got returncode %d'
                      % (cmd, p.returncode))

  def _StopProfiling(self, ssh_process):
    """Checks if the profiling process is alive and stops the process.

    Checks if the SSH process is alive. If the SSH process is alive, terminates
    the profiling process and returns true. If the SSH process is not alive, the
    profiling process has exited prematurely so returns false.
    """
    # Poll the SSH process to check if the connection is still alive. If it is
    # alive, the returncode should not be set.
    ssh_process.poll()
    if ssh_process.returncode is not None:
      logging.warning('Profiling process exited prematurely.')
      return False
    # Kill the profiling process directly. Terminating the SSH process doesn't
    # kill the profiling process.
    self._platform_backend.RunCommand(['killall', '-s', 'INT',
                                       self.PERF_BINARY_PATH])
    ssh_process.wait()
    return True

  @contextlib.contextmanager
  def SamplePeriod(self, period, action_runner, **_):
    """Collects CPU profiles for the giving period."""
    out_file = self.DEVICE_OUT_FILE_PATTERN.format(period=period)
    platform_backend = action_runner.tab.browser._platform_backend
    ssh_process = platform_backend.StartCommand(
        self._perf_command + ['-o', out_file])
    success = False
    try:
      yield
      success = True
    finally:
      success = self._StopProfiling(ssh_process) and success
      self._device_results.append((period, out_file, success))

  def _CreateArtifacts(self, file_safe_name, results):
    for period, device_file, ok in self._device_results:
      if not ok:
        continue
      with results.CaptureArtifact(
          'perf-%s-%s.perf.data' % (file_safe_name, period)) as dest_file:
        self._platform_backend.GetFile(device_file, dest_file)

  def GetResults(self, _, results):
    """Creates perf.data file artifacts from a successful story run."""
    if results.current_story_run.ok:
      # Benchmark and story names are delimited by "@@" and ends with "@@".
      # These can derived from the .perf.data filename.
      file_safe_name = (
          six.moves.urllib.parse.quote(results.benchmark_name, '')
          + "@@"
          + six.moves.urllib.parse.quote(results.current_story.name, '')
          + "@@")
      self._CreateArtifacts(file_safe_name, results)

    self._platform_backend.RunCommand(
        ['rm', '-f'] + [df for _, df, _ in self._device_results])
    self._device_results = []
