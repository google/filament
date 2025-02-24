# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import datetime
import glob
import heapq
import os
import subprocess
import time

import six

import dependency_manager  # pylint: disable=import-error

from telemetry.internal.util import local_first_binary_manager


def _ParseCrashpadDateTime(date_time_str):
  # Python strptime does not support time zone parsing, strip it.
  date_time_parts = date_time_str.split()
  if len(date_time_parts) >= 3:
    date_time_str = ' '.join(date_time_parts[:2])
  return datetime.datetime.strptime(date_time_str, '%Y-%m-%d %H:%M:%S')


class MinidumpFinder():
  """Handles finding Crashpad/Breakpad minidumps.

  In addition to whatever data is expected to be returned, most public methods
  also return a list of strings. These strings are what would normally be
  logged, but returned in the list instead of being logged directly to help
  cut down on log spam from uses such as
  BrowserBackend.GetRecentMinidumpPathWithTimeout().
  """
  def __init__(self, os_name, arch_name):
    super().__init__()
    self._os = os_name
    self._arch = arch_name
    self._minidump_path_crashpad_retrieval = {}
    self._explanation = []

  def MinidumpObtainedFromCrashpad(self, minidump):
    """Returns whether the given minidump was found via Crashpad or not.

    Args:
      minidump: the path to the minidump being checked.

    Returns:
      False if the dump was found via globbing, else True.
    """
    # Default to Crashpad where we hope to be eventually
    return self._minidump_path_crashpad_retrieval.get(minidump, True)

  def GetAllCrashpadMinidumps(self, minidump_dir):
    """Returns all minidumps in the given directory findable by Crashpad.

    Args:
      minidump_dir: The directory to look for minidumps in.

    Returns:
      A tuple. The first element is a list of paths to minidumps found by
      Crashpad, or None in the event of an error. The second element is a list
      of strings containing logging information generated while searching for
      the minidumps.
    """
    self._explanation = ['Attempting to find all Crashpad minidump files for a '
                         'suspected Chrome crash.']
    return self._GetAllCrashpadMinidumps(minidump_dir), self._explanation

  def GetAllMinidumpPaths(self, minidump_dir):
    """Finds all minidumps for either Crashpad or Breakpad.

    If any Crashpad minidumps are found, only they will be returned. Otherwise,
    Breakpad minidumps will be returned.

    Args:
      minidump_dir: The directory to look for minidumps in.

    Returns:
      A tuple. The first element is a list of paths to the found minidumps, or
      None in the event of an error. The second element is a list of strings
      containing logging information generated while searching for the
      minidumps.
    """
    self._explanation = ['Attempting to find all minidump files for a '
                         'suspected Chrome crash. Crashpad minidumps will be '
                         'searched for first, falling back to Breakpad '
                         'minidumps if none are found.']
    return self._GetAllMinidumpPaths(minidump_dir), self._explanation

  def GetMostRecentMinidump(self, minidump_dir):
    """Finds the most recently created Crashpad or Breakpad minidump.

    Args:
      minidump_dir: The directory to look for minidumps in.

    Returns:
      A tuple. The first element is a path to the found minidump, or None if
      no minidumps were found. The second element is a list of strings
      containing logging information generated while searching for the
      minidumps.
    """
    self._explanation = ['Attempting to find the most recent minidump file for '
                         'a suspected Chrome crash. Crashpad minidumps will be '
                         'searched for first, falling back to Breakpad '
                         'minidumps if none are found.']
    return self._GetMostRecentMinidump(minidump_dir), self._explanation

  def _GetAllCrashpadMinidumps(self, minidump_dir):
    if not minidump_dir:
      self._explanation.append('No minidump directory provided. Likely '
                               'attempted to retrieve the Crashpad minidumps '
                               'after the browser was already closed.')
      return None
    try:
      crashpad_database_util = \
          local_first_binary_manager.GetInstance().FetchPath(
              'crashpad_database_util')
      if not crashpad_database_util:
        self._explanation.append('Unable to find crashpad_database_util. This '
                                 'is likely due to running on a platform that '
                                 'does not support Crashpad.')
        return None
    except dependency_manager.NoPathFoundError:
      self._explanation.append(
          'Could not find a path to look for crashpad_database_util.')
      return None

    self._explanation.append(
        'Found crashpad_database_util at %s' % crashpad_database_util)

    report_output = subprocess.check_output([
        crashpad_database_util, '--database=' + minidump_dir,
        '--show-pending-reports', '--show-completed-reports',
        '--show-all-report-info'])
    # This can be removed once fully switched to Python 3 by setting text=True
    # when calling check_output above.
    if not isinstance(report_output, six.string_types):
      report_output = report_output.decode('utf-8')

    last_indentation = -1
    reports_list = []
    report_dict = {}
    for report_line in report_output.splitlines():
      # Report values are grouped together by the same indentation level.
      current_indentation = 0
      for report_char in report_line:
        if not report_char.isspace():
          break
        current_indentation += 1

      # Decrease in indentation level indicates a new report is being printed.
      if current_indentation >= last_indentation:
        report_key, report_value = report_line.split(':', 1)
        if report_value:
          report_dict[report_key.strip()] = report_value.strip()
      elif report_dict:
        try:
          report_time = _ParseCrashpadDateTime(report_dict['Creation time'])
          report_path = report_dict['Path'].strip()
          reports_list.append((report_time, report_path))
        except (ValueError, KeyError) as e:
          self._explanation.append('Expected to find keys "Path" and "Creation '
                                   'time" in Crashpad report, but one or both '
                                   'don\'t exist: %s' % e)
        finally:
          report_dict = {}

      last_indentation = current_indentation

    # Include the last report.
    if report_dict:
      try:
        report_time = _ParseCrashpadDateTime(report_dict['Creation time'])
        report_path = report_dict['Path'].strip()
        reports_list.append((report_time, report_path))
      except (ValueError, KeyError) as e:
        self._explanation.append('Expected to find keys "Path" and "Creation '
                                 'time" in Crashpad report, but one or both '
                                 'don\'t exist: %s' % e)

    return reports_list

  def _GetAllMinidumpPaths(self, minidump_dir):
    reports_list = self._GetAllCrashpadMinidumps(minidump_dir)
    if reports_list:
      for report in reports_list:
        self._minidump_path_crashpad_retrieval[report[1]] = True
      return [report[1] for report in reports_list]
    self._explanation.append('No minidumps found via crashpad_database_util, '
                             'falling back to globbing for Breakpad '
                             'minidumps.')
    dumps = self._GetBreakpadMinidumpPaths(minidump_dir)
    if dumps:
      self._explanation.append('Found Breakpad minidump via globbing.')
      for dump in dumps:
        self._minidump_path_crashpad_retrieval[dump] = False
      return dumps
    self._explanation.append(
        'Failed to find any Breakpad minidumps via globbing.')
    return []

  def _GetMostRecentCrashpadMinidump(self, minidump_dir):
    reports_list = self._GetAllCrashpadMinidumps(minidump_dir)
    if reports_list:
      _, most_recent_report_path = max(reports_list)
      return most_recent_report_path

    return None

  def _GetBreakpadMinidumpPaths(self, minidump_dir):
    if not minidump_dir:
      self._explanation.append('Attempted to fetch Breakpad minidump paths '
                               'without a minidump directory. The browser was '
                               'likely closed before attempting to fetch.')
      return None
    return glob.glob(os.path.join(minidump_dir, '*.dmp'))

  def _GetMostRecentMinidump(self, minidump_dir):
    # Crashpad dump layout will be the standard eventually, check it first.
    crashpad_dump = True
    most_recent_dump = self._GetMostRecentCrashpadMinidump(minidump_dir)

    # Typical breakpad format is simply dump files in a folder.
    if not most_recent_dump:
      crashpad_dump = False
      self._explanation.append('No minidump found via crashpad_database_util, '
                               'falling back to globbing for Breakpad '
                               'minidump.')
      dumps = self._GetBreakpadMinidumpPaths(minidump_dir)
      if dumps:
        most_recent_dump = heapq.nlargest(1, dumps, os.path.getmtime)[0]
        if most_recent_dump:
          self._explanation.append('Found Breakpad minidump via globbing.')

    # As a sanity check, make sure the crash dump is recent.
    if (most_recent_dump and
        os.path.getmtime(most_recent_dump) < (time.time() - (5 * 60))):
      self._explanation.append(
          'Crash dump is older than 5 minutes. May not be correct.')

    self._minidump_path_crashpad_retrieval[most_recent_dump] = crashpad_dump
    return most_recent_dump
