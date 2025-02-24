# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Utility methods for minidumps not related to finding or symbolizing.

Finding/symbolizing are handled in internal/backends/chrome/minidump_finder.py
and internal/chrome/backends/chrome/minidump_symbolizer.py respectively.
"""

import logging
import re
import subprocess
import sys

from telemetry.internal.util import local_first_binary_manager


def DumpMinidump(minidump_path):
  """Dumps a minidump's contents.

  Args:
    minidump_path: A filepath to a minidump.

  Returns:
    Either a string or None. In the case of a string, it contains the output of
    minidump_dump on the given minidump. None is returned if the minidump could
    not be dumped for some reason.
  """
  # minidump_dump does not exist for Windows, but dump_minidump_annotations can
  # dump similar information.
  cmd = []
  if sys.platform == 'win32':
    dump_annotations = local_first_binary_manager.GetInstance().FetchPath(
        'dump_minidump_annotations')
    if not dump_annotations:
      logging.warning(
          'No dump_minidump_annotations found, cannot dump %s', minidump_path)
      return None
    cmd = [dump_annotations, '--minidump', minidump_path]
  else:
    minidump_dump = local_first_binary_manager.GetInstance().FetchPath(
        'minidump_dump')
    if not minidump_dump:
      logging.warning('No minidump_dump found, cannot dump %s', minidump_path)
      return None
    cmd = [minidump_dump, minidump_path]
  proc = subprocess.run(cmd,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE,
                        text=True,
                        check=False)
  # We don't check the return code here because minidump_dump always (or at
  # least regularly) returns a non-zero code on success.
  if not proc.stdout:
    logging.warning('Dumping %s failed. minidump_dump stderr: %s',
                    minidump_path, proc.stderr)
    return None
  return proc.stdout


def GetCrashpadAnnotation(minidump_path, name, annotation_type=None):
  """Gets a Crashpad annotation value from a minidump.

  Args:
    minidump_path: A filepath to a minidump.
    name: A string containing the name of the Crashpad Annotation to get.
    annotation_type: An optional unsigned integer denoting the type of
        annotation that is expected. If not specified, will match any type.

  Returns:
    Either a string or None. In the case of a string, it contains the requested
    annotation value. None is returned if the value could not be retrieved.
  """
  if sys.platform == 'win32':
    if annotation_type:
      logging.warning(
          'Requested explicit annotation type %d, but dumping on Windows does '
          'not differentiate by type', annotation_type)
    # Looks for lines such as
    #   annotation_objects["ptype"] = gpu-process
    # and captures the value at the end (in this case, gpu-process)
    capture_string = (
        r'^\s*annotation_objects\[\"'
        + name
        + r'\"\] = (.*)$'
    )
    regex = re.compile(capture_string, re.MULTILINE)
  else:
    if annotation_type is None:
      annotation_type = r'\d+'
    # Looks for lines such as
    #   module_list[0].crashpad_annotations["ptype"] (type = 1) = gpu-process
    # and captures the value at the end (in this case, gpu-process).
    capture_string = (
        r'^\s*module_list\[\d+\]\.crashpad_annotations\[\"'
        + name
        + r'\"\] \(type = '
        + str(annotation_type)
        + r'\) = (.*)$')
    regex = re.compile(capture_string, re.MULTILINE)

  dump_output = DumpMinidump(minidump_path)
  if not dump_output:
    return None

  matched_groups = regex.findall(dump_output)
  if not matched_groups:
    logging.warning('Unable to find requested annotation %s in minidump', name)
    return None
  return matched_groups[0]


def GetProcessTypeFromMinidump(minidump_path):
  """Gets the process type of the crashed process from a minidump.

  Args:
    minidump_path: A filepath to a minidump.

  Returns:
    Either a string or None. In the case of a string, it contains the process
    type of the crashed process, e.g. "browser". None is returned if the process
    type cannot be determined.
  """
  return GetCrashpadAnnotation(minidump_path, 'ptype')
