# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A wrapper around ssh for common operations on a CrOS-based device"""
from __future__ import absolute_import
import logging
import re

from telemetry.core import linux_based_interface


class LinuxInterface(linux_based_interface.LinuxBasedInterface):

  X11_DISPLAY_DIR = '/tmp/.X11-unix'
  _REMOTE_USER = 'root'

  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self._hwinfo = {}
    self._xserver_proc = None
    self._current_display = None

  @property
  def display(self):
    return self._current_display

  def GetDisplays(self):
    stdout = self.RunCmdOnDevice(['ls', self.X11_DISPLAY_DIR])[0].strip()
    lock_files = self.RunCmdOnDevice(['ls',
                                      '/tmp/.X*-lock'])[0].strip().splitlines()
    displays = set()
    for display in stdout.splitlines():
      displays.add(int(display.replace('X', '')))
    for lock_file in lock_files:
      match = re.match(r'/tmp/.X(.*)-lock', lock_file)
      # Anything with an existing lock file is a stale, albeit existing
      # display.
      if match:
        displays.add(int(match.group(1)))
    return list(displays)

  def GetArchName(self):
    if self._arch_name is None:
      self._arch_name = self.RunCmdOnDevice(['uname', '-m'])[0].rstrip()
    return self._arch_name

  def LsbReleaseValue(self, key, default):
    """/etc/lsb-release is a file with key=value pairs."""
    lines = self.GetFileContents('/etc/lsb-release').split('\n')
    for l in lines:
      m = re.match(r'([^=]*)=(.*)', l)
      if m and m.group(1) == key:
        return m.group(2)
    return default

  def GetHardwareInfo(self, key):
    if not self._hwinfo.get(key, {}):
      contents = {}
      stdout = self.RunCmdOnDevice(['lshw'])[0].strip()
      add_contents = True
      tab_depth = -1
      for line in stdout.splitlines()[:1]:
        if not add_contents:
          break
        match = re.match(r'(\s+)(.*):\s+(.*)', line)
        # Matching tab depth determines correlated hardware properties.
        if match:
          if tab_depth == -1:
            tab_depth = len(match.group(1))
          elif tab_depth != len(match.group(1)):
            add_contents = False
            break
          contents[match.group(2)] = match.group(3)

      self._hwinfo = contents
    return self._hwinfo[key]

  def GetDeviceTypeName(self):
    """DEVICETYPE in /etc/lsb-release is CHROMEBOOK, CHROMEBIT, etc."""
    if self._device_type_name is None:
      self._device_type_name = self.GetHardwareInfo('description')
    return self._device_type_name

  def GetBoard(self):
    """Gets the name of the board of the device, e.g. "kevin".

    Returns:
      The name of the board as a string, or None if it can't be retrieved.
    """
    if self._board is None:
      self._board = self.GetHardwareInfo('product')
    return self._board

  def StopUI(self):
    if self._xserver_proc and self._xserver_proc.poll() is None:
      logging.debug('Terminating the previous Xserver')

      def MatchXServer(cmd: str):
        return re.match(r'.*Xorg :' + str(self._current_display), cmd)

      self.KillAllMatching(MatchXServer)
      self._xserver_proc.terminate()
    self.RmRF(self.path.join(self.X11_DISPLAY_DIR, f'X{self._current_display}'))
    self.RmRF(self.path.join('/tmp', f'.X{self._current_display}-lock'))
    self._xserver_proc = None

  def RestartUI(self):
    logging.info('(Re)starting the ui')
    self.StopUI()
    displays = self.GetDisplays()
    new_display = displays[-1] + 1
    start_cmd = ['xinit', '--', f':{new_display}']
    self._xserver_proc = self.StartCmdOnDevice(start_cmd)
    self._current_display = new_display

  def CloseConnection(self):
    self.StopUI()
    super().CloseConnection()
