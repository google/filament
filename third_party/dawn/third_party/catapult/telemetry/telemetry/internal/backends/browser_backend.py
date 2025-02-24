# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import division
from __future__ import absolute_import
import logging
import os
import posixpath
import uuid
import re
import sys
import tempfile
import threading
import time

from datetime import datetime
from py_utils import cloud_storage  # pylint: disable=import-error

from telemetry import decorators
from telemetry.core import debug_data
from telemetry.core import exceptions
from telemetry.internal.backends import app_backend
from telemetry.internal.browser import web_contents
from telemetry.internal.results import artifact_logger
from telemetry.util import screenshot


# Looks for "Thread X (crashed)", where X is an integer
CRASHED_THREAD_HEADER_REGEX = re.compile(
    r'^\s*Thread \d+ \(crashed\)\s*$', re.MULTILINE)
# Looks for a hexadecimal RetAddr followed by several hexadecimal arguments
# that come before the call site/function name.
WINDOWS_STACK_FRAME_REGEX = re.compile(
    # Optional leading whitespace.
    r'^\s*'
    # RetAddr.
    r'[\dabcdef]{8}`[\dabcdef]{8} : '
    # Args to child.
    r'[\dabcdef]{8}`[\dabcdef]{8} [\dabcdef]{8}`[\dabcdef]{8} '
    r'[\dabcdef]{8}`[\dabcdef]{8} [\dabcdef]{8}`[\dabcdef]{8} : '
    # Call Site.
    r'(.*)$', re.MULTILINE)
STACK_FRAMES_PER_SUMMARY = 3
# Stack frames that we omit from stack summaries, likely because they are not
# descriptive of the actual issue.
OMITTED_STACK_SUMMARY_FRAMES = frozenset([
    'libc.so.6',
])


class ExtensionsNotSupportedException(Exception):
  pass


class BrowserBackend(app_backend.AppBackend):
  """A base class for browser backends."""

  def __init__(self, platform_backend, browser_options,
               supports_extensions, tab_list_backend):
    assert browser_options.browser_type
    super().__init__(browser_options.browser_type,
                                         platform_backend)
    self.browser_options = browser_options
    self._supports_extensions = supports_extensions
    self._tab_list_backend_class = tab_list_backend
    self._dump_finder = None
    self._tmp_minidump_dir = tempfile.mkdtemp()
    self._symbolized_minidump_paths = set([])
    self._periodic_screenshot_timer = None
    self._collect_periodic_screenshots = False

  def SetBrowser(self, browser):
    super().SetApp(app=browser)

  @property
  def log_file_path(self):
    # Specific browser backend is responsible for overriding this properly.
    raise NotImplementedError

  def GetLogFileContents(self):
    if not self.log_file_path:
      return 'No log file'
    with open(self.log_file_path) as f:
      return f.read()

  def UploadLogsToCloudStorage(self):
    """ Uploading log files produce by this browser instance to cloud storage.

    Check supports_uploading_logs before calling this method.
    """
    assert self.supports_uploading_logs
    remote_path = (self.browser_options.logs_cloud_remote_path or
                   'log_%s' % uuid.uuid4())
    cloud_url = cloud_storage.Insert(
        bucket=self.browser_options.logs_cloud_bucket,
        remote_path=remote_path,
        local_path=self.log_file_path)
    sys.stderr.write('Uploading browser log to %s\n' % cloud_url)

  @property
  def browser(self):
    return self.app

  @property
  def browser_type(self):
    return self.app_type

  @property
  def screenshot_timeout(self):
    return None

  @property
  def supports_uploading_logs(self):
    # Specific browser backend is responsible for overriding this properly.
    return False

  @property
  def supports_extensions(self):
    """True if this browser backend supports extensions."""
    return self._supports_extensions

  @property
  def supports_tab_control(self):
    raise NotImplementedError()

  @property
  @decorators.Cache
  def tab_list_backend(self):
    return self._tab_list_backend_class(self)

  @property
  def supports_app_ui_interactions(self):
    return False

  def Start(self, startup_args):
    raise NotImplementedError()

  def IsBrowserRunning(self):
    raise NotImplementedError()

  def IsAppRunning(self):
    return self.IsBrowserRunning()

  def GetStandardOutput(self):
    raise NotImplementedError()

  def PullMinidumps(self):
    """Pulls any minidumps off a test device if necessary."""

  def CollectDebugData(self, log_level):
    """Collects various information that may be useful for debugging.

    Specifically:
      1. Captures a screenshot.
      2. Collects stdout and system logs.
      3. Attempts to symbolize all currently unsymbolized minidumps.

    All collected information is stored as artifacts, and everything but the
    screenshot is also included in the return value.

    Platforms may override this to provide other debug information in addition
    to the above set of information.

    Args:
      log_level: The logging level to use from the logging module, e.g.
          logging.ERROR.

    Returns:
      A debug_data.DebugData object containing the collected data.
    """
    suffix = artifact_logger.GetTimestampSuffix()
    data = debug_data.DebugData()
    self._CollectScreenshot(log_level, suffix + '.png')
    self._CollectSystemLog(log_level, suffix + '.txt', data)
    self._CollectStdout(log_level, suffix + '.txt', data)
    self._SymbolizeAndLogMinidumps(log_level, data)
    return data

  def StartCollectingPeriodicScreenshots(self, frequency_ms):
    self._collect_periodic_screenshots = True
    self._CollectPeriodicScreenshots(datetime.now(), frequency_ms)

  def StopCollectingPeriodicScreenshots(self):
    self._collect_periodic_screenshots = False
    self._periodic_screenshot_timer.cancel()

  def _CollectPeriodicScreenshots(self, start_time, frequency_ms):
    self._CollectScreenshot(logging.INFO, "periodic.png", start_time)
    #2To3-division: this line is unchanged as result is expected floats.
    self._periodic_screenshot_timer = threading.Timer(
        frequency_ms / 1000.0,
        self._CollectPeriodicScreenshots,
        [start_time, frequency_ms])
    if self._collect_periodic_screenshots:
      self._periodic_screenshot_timer.start()

  def _CollectScreenshot(self, log_level, suffix, start_time=None):
    """Helper function to handle the screenshot portion of CollectDebugData.

    Attempts to take a screenshot at the OS level and save it as an artifact.

    Args:
      log_level: The logging level to use from the logging module, e.g.
          logging.ERROR.
      suffix: The suffix to prepend to the names of any created artifacts.
      start_time: If set, prepend elapsed time to screenshot path.
          Should be time at which the test started, as a datetime.
          This is done here because it may take a nonzero amount of time
          to take a screenshot.
    """
    screenshot_handle = screenshot.TryCaptureScreenShot(
        self.browser.platform, timeout=self.screenshot_timeout)
    if screenshot_handle:
      with open(screenshot_handle.GetAbsPath(), 'rb') as infile:
        if start_time:
          # Prepend time since test started to path
          test_time = datetime.now() - start_time
          suffix = str(test_time.total_seconds()).replace(
              '.', '_') + '-' + suffix

        artifact_name = posixpath.join(
            'debug_screenshots', 'screenshot-%s' % suffix)
        logging.log(
            log_level, 'Saving screenshot as artifact %s', artifact_name)
        artifact_logger.CreateArtifact(artifact_name, infile.read())
    else:
      logging.log(log_level, 'Failed to capture screenshot')

  def _CollectSystemLog(self, log_level, suffix, data):
    """Helper function to handle the system log part of CollectDebugData.

    Attempts to retrieve the system log, save it as an artifact, and add it to
    the given DebugData object.

    Args:
      log_level: The logging level to use from the logging module, e.g.
          logging.ERROR.
      suffix: The suffix to append to the names of any created artifacts.
      data: The debug_data.DebugData object to add collected data to.
    """
    system_log = self.browser.platform.GetSystemLog()
    if system_log is None:
      logging.log(log_level, 'Platform did not provide a system log')
      return
    artifact_name = posixpath.join('system_logs', 'system_log-%s' % suffix)
    logging.log(log_level, 'Saving system log as artifact %s', artifact_name)
    artifact_logger.CreateArtifact(artifact_name, system_log)
    data.system_log = system_log

  def _CollectStdout(self, log_level, suffix, data):
    """Helper function to handle the stdout part of CollectDebugData.

    Attempts to retrieve stdout, save it as an artifact, and add it to the given
    DebugData object.

    Args:
      log_level: The logging level to use from the logging module, e.g.
          logging.ERROR.
      suffix: The suffix to append to the names of any created artifacts.
      data: The debug_data.DebugData object to add collected data to.
    """
    stdout = self.browser.GetStandardOutput()
    if stdout is None:
      logging.log(log_level, 'Browser did not provide stdout')
      return
    artifact_name = posixpath.join('stdout', 'stdout-%s' % suffix)
    logging.log(log_level, 'Saving stdout as artifact %s', artifact_name)
    artifact_logger.CreateArtifact(artifact_name, stdout)
    data.stdout = stdout

  def _SymbolizeAndLogMinidumps(self, log_level, data):
    """Helper function to handle the minidump portion of CollectDebugData.

    Attempts to find all unsymbolized minidumps, symbolize them, save the
    results as artifacts, add them to the given DebugData object, and log the
    results.

    Args:
      log_level: The logging level to use from the logging module, e.g.
          logging.ERROR.
      data: The debug_data.DebugData object to add collected data to.
    """
    paths = self.GetAllUnsymbolizedMinidumpPaths()
    # It's probable that CollectDebugData() is being called in response to a
    # crash. Minidumps are usually written to disk in time, but there's no
    # guarantee that is the case. So, if we don't find any minidumps, poll for
    # a bit to ensure we don't miss them.
    if not paths:
      self.browser.GetRecentMinidumpPathWithTimeout(5)
      paths = self.GetAllUnsymbolizedMinidumpPaths()
    if not paths:
      logging.log(log_level, 'No unsymbolized minidump paths')
      return
    logging.log(log_level, 'Unsymbolized minidump paths: ' + str(paths))
    for unsymbolized_path in paths:
      minidump_name = os.path.basename(unsymbolized_path)
      artifact_name = posixpath.join('unsymbolized_minidumps', minidump_name)
      logging.log(log_level, 'Saving minidump as artifact %s', artifact_name)
      with open(unsymbolized_path, 'rb') as infile:
        artifact_logger.CreateArtifact(artifact_name, infile.read())
      valid, output = self.SymbolizeMinidump(unsymbolized_path)
      # Store the symbolization attempt as an artifact.
      artifact_name = posixpath.join('symbolize_attempts', minidump_name)
      logging.log(log_level, 'Saving symbolization attempt as artifact %s',
                  artifact_name)
      artifact_logger.CreateArtifact(artifact_name, output)
      if valid:
        logging.log(log_level, 'Symbolized minidump:\n%s', output)
        data.symbolized_minidumps.append(output)
      else:
        logging.log(
            log_level,
            'Minidump symbolization failed, check artifact %s for output',
            artifact_name)

  def CleanupUnsymbolizedMinidumps(self, fatal=False):
    """Cleans up any unsymbolized minidumps so they aren't found later.

    Args:
      fatal: Whether the presence of unsymbolized minidumps should be considered
          a fatal error or not. Typically, before a test should be non-fatal,
          while after a test should be fatal.
    """
    log_level = logging.ERROR if fatal else logging.WARNING
    unsymbolized_paths = self.GetAllUnsymbolizedMinidumpPaths(log=False)
    if not unsymbolized_paths:
      return

    culprit_test = 'current test' if fatal else 'a previous test'
    logging.log(log_level,
                'Found %d unsymbolized minidumps leftover from %s. Outputting '
                'below: ', len(unsymbolized_paths), culprit_test)
    dd = self.CollectDebugData(log_level)
    if fatal:
      # Try to include the files/function names of the first couple stack frames
      # in the error. This can help users see what the issue is at a glance and
      # also helps automated tooling differentiate between different root
      # causes for this error.
      stack_summaries = _GetStackSummaries(dd.symbolized_minidumps)
      string_summaries = []
      for s in stack_summaries:
        if s is None:
          string_summaries.append('<unparsable stack>')
        else:
          # Flip the order since the summary starts with the crashing frame, but
          # we want to report the functions in the order that they were called.
          s.reverse()
          string_summaries.append(' > '.join(s))
      raise RuntimeError(
          'Test left %d unsymbolized minidumps around after finishing. Stack '
          'summaries: %s' % (len(unsymbolized_paths),
                             ', '.join(string_summaries)))

  def IgnoreMinidump(self, path):
    """Ignores the given minidump, treating it as already symbolized.

    Args:
      path: The path to the minidump to ignore.
    """
    self._symbolized_minidump_paths.add(path)

  def GetMostRecentMinidumpPath(self):
    """Gets the most recent minidump that has been written to disk.

    Returns:
      The path to the most recent minidump on disk, or None if no minidumps are
      found.
    """
    self.PullMinidumps()
    dump_path, explanation = self._dump_finder.GetMostRecentMinidump(
        self._tmp_minidump_dir)
    logging.info('\n'.join(explanation))
    return dump_path

  def GetRecentMinidumpPathWithTimeout(self, timeout_s, oldest_ts):
    """Get a path to a recent minidump, blocking until one is available.

    Similar to GetMostRecentMinidumpPath, but does not assume that any pending
    dumps have been written to disk yet. Instead, waits until a suitably fresh
    minidump is found or the timeout is reached.

    Args:
      timeout_s: The timeout in seconds.
      oldest_ts: The oldest allowable timestamp (in seconds since epoch) that a
          minidump was created at for it to be considered fresh enough to
          return. Defaults to a minute from the current time if not set.

    Returns:
      None if the timeout is hit or a str containing the path to the found
      minidump if a suitable one is found.
    """
    assert timeout_s > 0
    assert oldest_ts >= 0
    explanation = ['No explanation returned.']
    start_time = time.time()
    try:
      while time.time() - start_time < timeout_s:
        self.PullMinidumps()
        dump_path, explanation = self._dump_finder.GetMostRecentMinidump(
            self._tmp_minidump_dir)
        if not dump_path or os.path.getmtime(dump_path) < oldest_ts:
          continue
        return dump_path
      return None
    finally:
      logging.info('\n'.join(explanation))

  def GetAllMinidumpPaths(self, log=True):
    """Get all paths to minidumps currently written to disk.

    Args:
      log: Whether to log the output from looking for minidumps or not.

    Returns:
      A list of paths to all found minidumps.
    """
    self.PullMinidumps()
    paths, explanation = self._dump_finder.GetAllMinidumpPaths(
        self._tmp_minidump_dir)
    if log:
      logging.info('\n'.join(explanation))
    return paths

  def GetAllUnsymbolizedMinidumpPaths(self, log=True):
    """Get all paths to minidumps have have not yet been symbolized.

    Args:
      log: Whether to log the output from looking for minidumps or not.

    Returns:
      A list of paths to all found minidumps that have not been symbolized yet.
    """
    minidump_paths = set(self.GetAllMinidumpPaths(log=log))
    # If we have already symbolized paths remove them from the list
    unsymbolized_paths = (
        minidump_paths - self._symbolized_minidump_paths)
    return list(unsymbolized_paths)

  def SymbolizeMinidump(self, minidump_path):
    """Symbolizes the given minidump.

    Args:
      minidump_path: The path to the minidump to symbolize.

    Returns:
      A tuple (valid, output). |valid| is True if the minidump was symbolized,
      otherwise False. |output| contains an error message if |valid| is False,
      otherwise it contains the symbolized minidump.
    """
    raise NotImplementedError()

  def GetSystemInfo(self):
    return None

  def GetVersionInfo(self):
    return {}

  @property
  def supports_memory_dumping(self):
    return False

  def DumpMemory(self, timeout=None, detail_level=None, deterministic=False):
    raise NotImplementedError()

# pylint: disable=invalid-name
  @property
  def supports_overriding_memory_pressure_notifications(self):
    return False

  def SetMemoryPressureNotificationsSuppressed(
      self, suppressed, timeout=web_contents.DEFAULT_WEB_CONTENTS_TIMEOUT):
    raise NotImplementedError()

  def SimulateMemoryPressureNotification(
      self, pressure_level, timeout=web_contents.DEFAULT_WEB_CONTENTS_TIMEOUT):
    raise NotImplementedError()

  @property
  def supports_cpu_metrics(self):
    raise NotImplementedError()

  @property
  def supports_memory_metrics(self):
    raise NotImplementedError()

  @property
  def supports_overview_mode(self): # pylint: disable=invalid-name
    return False

  def EnterOverviewMode(self, timeout): # pylint: disable=unused-argument
    raise exceptions.StoryActionError('Overview mode is not supported')

  def ExitOverviewMode(self, timeout): # pylint: disable=unused-argument
    raise exceptions.StoryActionError('Overview mode is not supported')

  def ExecuteBrowserCommand(
      self, command_id, timeout): # pylint: disable=unused-argument
    raise exceptions.StoryActionError('Execute browser command not supported')

  def SetDownloadBehavior(
      self, behavior, downloadPath, timeout): # pylint: disable=unused-argument
    raise exceptions.StoryActionError('Set download behavior not supported')

  def GetUIDevtoolsBackend(self):
    raise exceptions.StoryActionError('UI Devtools not supported')

  def GetWindowForTarget(self, target_id): # pylint: disable=unused-argument
    raise exceptions.StoryActionError('Get Window For Target not supported')

  def SetWindowBounds(
      self, window_id, bounds): # pylint: disable=unused-argument
    raise exceptions.StoryActionError('Set Window Bounds not supported')


def _GetStackSummaries(symbolized_minidumps):
  """Attempts to summarize the most relevant parts of the given minidumps.

  As an example, if one of the given minidumps' crash was due to the function
  calls A -> B -> C where C is the crashing function, then the summary for that
  minidump/stack would be ['C', 'B', 'A'].

  Args:
    symbolized_minidumps: A list of strings, each string containing a
        symbolized minidump.

  Returns:
    A list of either lists of strings or None. The element at index i
    corresponds to the ith element of |symbolized_minidumps|. A None element
    indicates that a stack summary could not be generated for some reason. A
    list of strings element contains the stack summary in stack order, i.e. the
    first element is the crashing function.
  """
  stack_summaries = []
  for sm in symbolized_minidumps:
    # Windows has a unique stack format that must be handled separately.
    # Realistically we would never have a mix of different stack types, but
    # there isn't a technical reason why we shouldn't be able to parse both at
    # the same time.
    if WINDOWS_STACK_FRAME_REGEX.search(sm):
      stack_summaries.append(_GetWindowsStackSummary(sm))
      continue

    # Symbolized minidumps consist of the following in the given order:
    #   * A header containing device information, crash reason, etc.
    #   * A stack for the crashed thread with the header "Thread X (crashed)"
    #   * Stacks for non-crashed threads
    # So, start by chopping off the header so we can look at the crashed thread.
    matches = CRASHED_THREAD_HEADER_REGEX.findall(sm)
    if not matches:
      logging.info('Unable to find crashed thread header')
      stack_summaries.append(None)
      continue
    start_index = sm.find(matches[0])
    if start_index == -1:
      logging.info('Unable to find index of crashed thread header')
      stack_summaries.append(None)
      continue
    sm = sm[start_index:]

    # Now look for the first X stack frames, whose first line is in the
    # following format when symbolized:
    #   frame_number file!namespace::function(arg_types...) + offset
    # or the following format when not symbolized
    #   frame_number file + offset
    # In the former case, we just want everything up through the function, but
    # not the argument types (for brevity). In the latter case, we want just the
    # file name.
    frames = []
    for line in sm.splitlines():
      line = line.strip()
      if not line:
        continue
      split_line = line.split(maxsplit=1)
      if not split_line[0].isdigit():
        # No frame number, line isn't of interest.
        continue
      function_or_filename = split_line[1]
      # It's possible for the filename to have a space, so look for a + and
      # assume everything before that is the filename + function. If a + is not
      # found, then assume that there is no offset and we care about everything.
      # The lack of offset is rare but possible, e.g. if the crash involves the
      # kernel.
      if '+' in function_or_filename:
        function_or_filename = function_or_filename.split('+')[0].rstrip()
      if '(' in function_or_filename:
        # Actual function with argument types.
        function_or_filename = function_or_filename.split('(')[0]
      if function_or_filename not in OMITTED_STACK_SUMMARY_FRAMES:
        frames.append(function_or_filename)
      if len(frames) == STACK_FRAMES_PER_SUMMARY:
        break

    if not frames:
      logging.info('Unable to parse any stack frames')
      stack_summaries.append(None)
      continue
    stack_summaries.append(frames)

  return stack_summaries


def _GetWindowsStackSummary(symbolized_minidump):
  """Helper for getting a stack summary for a Windows stack summary.

  Windows minidumps get symbolized to a different format than ones from
  Unix-like OSes.

  Args:
    symbolized_minidump: A string containing a symbolized Windows minidump.

  Returns:
    A list of strings or None. None indicates that a stack summary could not be
    generated for some reason. A list of strings contains the stack summary
    in stack order, i.e. the first element is the crashing function.
  """
  call_sites = []
  for call_site in WINDOWS_STACK_FRAME_REGEX.findall(symbolized_minidump):
    # If there is an offset, strip that off. Otherwise, append the entire call
    # site.
    if '+' in call_site:
      call_sites.append(call_site.split('+')[0])
    else:
      call_sites.append(call_site)
    if len(call_sites) == STACK_FRAMES_PER_SUMMARY:
      break

  if not call_sites:
    logging.info('Unable to parse any call sites')
    return None
  return call_sites
