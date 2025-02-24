# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry import decorators
from telemetry.internal.platform import platform_backend


class LinuxBasedPlatformBackend(platform_backend.PlatformBackend):

  """Abstract platform containing functionality shared by all Linux based OSes.

  This includes Android and ChromeOS.

  Subclasses must implement RunCommand, GetFileContents."""

  @decorators.Cache
  def GetSystemTotalPhysicalMemory(self):
    meminfo_contents = self.GetFileContents('/proc/meminfo')
    meminfo = self._GetProcFileDict(meminfo_contents)
    if not meminfo:
      return None
    return self._ConvertToBytes(meminfo['MemTotal'])

  def GetFileContents(self, filename):
    raise NotImplementedError()

  def RunCommand(self, cmd):
    """Runs the specified command.

    Args:
        cmd: A list of program arguments or the path string of the program.
    Returns:
        A string whose content is the output of the command.
    """
    raise NotImplementedError()

  def _ConvertToBytes(self, value):
    return int(value.replace('kB', '')) * 1024

  def _GetProcFileDict(self, contents):
    retval = {}
    for line in contents.splitlines():
      key, value = line.split(':')
      retval[key.strip()] = value.strip()
    return retval
