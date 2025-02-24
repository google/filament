# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import division
from __future__ import absolute_import
import contextlib
import ctypes
import logging
import os
import platform
import re
import subprocess
import sys
import six
from PIL import ImageGrab  # pylint: disable=import-error

from telemetry.core import exceptions
from telemetry.core import os_version as os_version_module
from telemetry import decorators
from telemetry.internal.platform import desktop_platform_backend

try:
  import pywintypes  # pylint: disable=import-error
  import win32api  # pylint: disable=import-error
  from win32com.shell import shell  # pylint: disable=no-name-in-module
  from win32com.shell import shellcon  # pylint: disable=no-name-in-module
  import win32con  # pylint: disable=import-error
  import win32gui  # pylint: disable=import-error
  import win32process  # pylint: disable=import-error
  import winerror  # pylint: disable=import-error
  import win32security  # pylint: disable=import-error
except ImportError as e:
  if platform.system() == 'Windows':
    logging.warning('import error in win_platform_backend: %s', e)
  pywintypes = None
  shell = None
  shellcon = None
  win32api = None
  win32con = None
  win32file = None
  win32gui = None
  win32pipe = None
  win32process = None
  win32security = None
  winerror = None


class WinPlatformBackend(desktop_platform_backend.DesktopPlatformBackend):
  def __init__(self):
    super().__init__()

  @classmethod
  def IsPlatformBackendForHost(cls):
    return sys.platform == 'win32'

  def IsThermallyThrottled(self):
    raise NotImplementedError()

  def HasBeenThermallyThrottled(self):
    raise NotImplementedError()

  @decorators.Cache
  def GetSystemTotalPhysicalMemory(self):
    performance_info = self._GetPerformanceInfo()
    return performance_info.PhysicalTotal * performance_info.PageSize // 1024

  def KillProcess(self, pid, kill_process_tree=False):
    # os.kill for Windows is Python 2.7.
    cmd = ['taskkill', '/F', '/PID', str(pid)]
    if kill_process_tree:
      cmd.append('/T')
    subprocess.Popen(cmd, stdout=subprocess.PIPE,
                     stderr=subprocess.STDOUT).communicate()

  def GetSystemProcessInfo(self):
    # [3:] To skip 2 blank lines and header.
    lines = six.ensure_str(
        subprocess.Popen(
            ['wmic', 'process', 'get',
             'CommandLine,CreationDate,Name,ParentProcessId,ProcessId',
             '/format:csv'],
            stdout=subprocess.PIPE).communicate()[0]
        ).splitlines()[3:]
    process_info = []
    for line in lines:
      if not line:
        continue
      parts = line.split(',')
      pi = {}
      pi['ProcessId'] = int(parts[-1])
      pi['ParentProcessId'] = int(parts[-2])
      pi['Name'] = parts[-3]
      creation_date = None
      if parts[-4]:
        creation_date = float(re.split('[+-]', parts[-4])[0])
      pi['CreationDate'] = creation_date
      pi['CommandLine'] = ','.join(parts[1:-4])
      process_info.append(pi)
    return process_info

  @decorators.Cache
  def GetPcSystemType(self):
    # WMIC was introduced in Windows 2000, deprecated in Windows 10 21H1 (build
    # 19043), and removed in Windows 10 22H1. Get-CimInstance is the recommended
    # replacement, introduced in PowerShell 3.0, together with Windows 8. So to
    # work with OSes starting from Windows 7, we need to keep them both.
    # Details about computer system can be found at
    # https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-computersystem

    use_powershell = int(platform.version().split('.')[-1]) >= 19043

    if use_powershell:
      args = ['powershell', 'Get-CimInstance -ClassName Win32_ComputerSystem' \
              ' | Select-Object -Property PCSystemType']
    else:
      args = ['wmic', 'computersystem', 'get', 'pcsystemtype']

    # Retry this up to 3 times. On Windows ARM64 devices, it is unlikely but
    # possible for the powershell command to hang indefinitely. The
    # -OperationTimeoutSec argument for the Get-CimInstance command does not
    # prevent this.
    lines = []
    for _ in range(3):
      try:
        proc = subprocess.run(
            args, text=True, timeout=10, check=True, capture_output=True)
        lines = proc.stdout.split()
        break
      except subprocess.CalledProcessError as e:
        logging.error('Error running %s: %s', args, e)
      except subprocess.TimeoutExpired as e:
        logging.error('Timeout running %s: %s', args, e)

    if len(lines) > 1 and lines[0] == 'PCSystemType':
      if use_powershell:
        return lines[2]
      return lines[1]
    return '0'

  def IsLaptop(self):
    # if the pcsystemtype value is 2, then it is mobile/laptop.
    return self.GetPcSystemType() == '2'

  def GetTypExpectationsTags(self):
    tags = super().GetTypExpectationsTags()
    if self.IsLaptop():
      tags.append('win-laptop')
    return tags

  @decorators.Cache
  def GetArchName(self):
    return platform.machine()

  def GetOSName(self):
    return 'win'

  @decorators.Cache
  def GetOSVersionName(self):
    _MIN_WIN11_BUILD = 22000

    os_version = platform.uname()[3]

    if os_version.startswith('5.1.'):
      return os_version_module.XP
    if os_version.startswith('6.0.'):
      return os_version_module.VISTA
    if os_version.startswith('6.1.'):
      return os_version_module.WIN7
    if os_version.startswith('6.2.'):
      return os_version_module.WIN8
    if os_version.startswith('6.3.'):
      return os_version_module.WIN81
    if os_version.startswith('10.0.'):
      build = os_version.split('.')[2]
      if int(build) >= _MIN_WIN11_BUILD:
        return os_version_module.WIN11
      return os_version_module.WIN10
    raise NotImplementedError('Unknown win version: %s' % os_version)

  @decorators.Cache
  def GetOSVersionDetailString(self):
    return platform.uname()[3]

  def CanTakeScreenshot(self):
    return True

  def TakeScreenshot(self, file_path):
    image = ImageGrab.grab(all_screens=True, include_layered_windows=True)
    with open(file_path, 'wb') as f:
      image.save(f, 'PNG')
    return True

  def GetScreenResolution(self):
    # SM_CXSCREEN - width of the screen of the primary display monitor
    # SM_CYSCREEN - height of the screen of the primary display monitor
    # resolution returned by GetSystemMetrics is scaled
    width = ctypes.windll.user32.GetSystemMetrics(win32con.SM_CXSCREEN)
    height = ctypes.windll.user32.GetSystemMetrics(win32con.SM_CYSCREEN)

    if self.GetOSVersionName() < os_version_module.WIN81:
      # shcore.dll first introduced in Windows 8.1
      return width, height

    # 0 - DEVICE_PRIMARY (primary display monitor)
    scale = ctypes.windll.shcore.GetScaleFactorForDevice(0)
    return width * scale // 100, height * scale // 100

  def CanFlushIndividualFilesFromSystemCache(self):
    return True

  def _GetWin32ProcessInfo(self, func, pid):
    mask = (win32con.PROCESS_QUERY_INFORMATION |
            win32con.PROCESS_VM_READ)
    handle = None
    try:
      handle = win32api.OpenProcess(mask, False, pid)
      return func(handle)
    except pywintypes.error as e:
      if e.winerror == winerror.ERROR_INVALID_PARAMETER:
        raise exceptions.ProcessGoneException()
      raise
    finally:
      if handle:
        win32api.CloseHandle(handle)

  def _GetPerformanceInfo(self):
    class PerformanceInfo(ctypes.Structure):
      """Struct for GetPerformanceInfo() call
      http://msdn.microsoft.com/en-us/library/ms683210
      """
      _fields_ = [('size', ctypes.c_ulong),
                  ('CommitTotal', ctypes.c_size_t),
                  ('CommitLimit', ctypes.c_size_t),
                  ('CommitPeak', ctypes.c_size_t),
                  ('PhysicalTotal', ctypes.c_size_t),
                  ('PhysicalAvailable', ctypes.c_size_t),
                  ('SystemCache', ctypes.c_size_t),
                  ('KernelTotal', ctypes.c_size_t),
                  ('KernelPaged', ctypes.c_size_t),
                  ('KernelNonpaged', ctypes.c_size_t),
                  ('PageSize', ctypes.c_size_t),
                  ('HandleCount', ctypes.c_ulong),
                  ('ProcessCount', ctypes.c_ulong),
                  ('ThreadCount', ctypes.c_ulong)]

      def __init__(self):
        self.size = ctypes.sizeof(self)
        # pylint: disable=bad-super-call
        super().__init__()

    performance_info = PerformanceInfo()
    ctypes.windll.psapi.GetPerformanceInfo(
        ctypes.byref(performance_info), performance_info.size)
    return performance_info

  def IsCurrentProcessElevated(self):
    if self.GetOSVersionName() < os_version_module.VISTA:
      # TOKEN_QUERY is not defined before Vista. All processes are elevated.
      return True

    handle = win32process.GetCurrentProcess()
    with contextlib.closing(
        win32security.OpenProcessToken(handle, win32con.TOKEN_QUERY)) as token:
      return bool(win32security.GetTokenInformation(
          token, win32security.TokenElevation))

  def LaunchApplication(
      self, application, parameters=None, elevate_privilege=False):
    """Launch an application. Returns a PyHANDLE object."""

    parameters = ' '.join(parameters) if parameters else ''
    if elevate_privilege and not self.IsCurrentProcessElevated():
      # Use ShellExecuteEx() instead of subprocess.Popen()/CreateProcess() to
      # elevate privileges. A new console will be created if the new process has
      # different permissions than this process.
      proc_info = shell.ShellExecuteEx(
          fMask=shellcon.SEE_MASK_NOCLOSEPROCESS | shellcon.SEE_MASK_NO_CONSOLE,
          lpVerb='runas' if elevate_privilege else '',
          lpFile=application,
          lpParameters=parameters,
          nShow=win32con.SW_HIDE)
      if proc_info['hInstApp'] <= 32:
        raise Exception('Unable to launch %s' % application)
      return proc_info['hProcess']
    handle, _, _, _ = win32process.CreateProcess(None,
                                                 application + ' ' + parameters,
                                                 None, None, False,
                                                 win32process.CREATE_NO_WINDOW,
                                                 None, None,
                                                 win32process.STARTUPINFO())
    return handle

  def IsCooperativeShutdownSupported(self):
    return True

  def CooperativelyShutdown(self, proc, app_name):
    if win32gui is None:
      logging.warning('win32gui unavailable, cannot cooperatively shutdown')
      return False

    pid = proc.pid

    # http://timgolden.me.uk/python/win32_how_do_i/
    #   find-the-window-for-my-subprocess.html
    #
    # It seems that intermittently this code manages to find windows
    # that don't belong to Chrome -- for example, the cmd.exe window
    # running slave.bat on the tryservers. Try to be careful about
    # finding only Chrome's windows. This works for both the browser
    # and content_shell.
    #
    # It seems safest to send the WM_CLOSE messages after discovering
    # all of the sub-process's windows.
    def find_chrome_windows(hwnd, hwnds):
      try:
        _, win_pid = win32process.GetWindowThreadProcessId(hwnd)
        if (pid == win_pid and
            win32gui.IsWindowVisible(hwnd) and
            win32gui.IsWindowEnabled(hwnd) and
            win32gui.GetClassName(hwnd).lower().startswith(app_name)):
          hwnds.append(hwnd)
      except pywintypes.error as e:
        # Some windows may close after enumeration and before the calls above,
        # so ignore those.
        if e.winerror != winerror.ERROR_INVALID_WINDOW_HANDLE:
          raise
      return True
    hwnds = []
    win32gui.EnumWindows(find_chrome_windows, hwnds)
    if hwnds:
      for hwnd in hwnds:
        win32gui.SendMessage(hwnd, win32con.WM_CLOSE, 0, 0)
      return True
    logging.info('Did not find any windows owned by target process')
    return False

  def GetIntelPowerGadgetPath(self):
    ipg_dir = os.getenv('IPG_Dir')
    if not ipg_dir:
      logging.debug('No env IPG_Dir')
      return None
    gadget_path = os.path.join(ipg_dir, 'PowerLog3.0.exe')
    if not os.path.isfile(gadget_path):
      logging.debug('Cannot locate Intel Power Gadget at ' + gadget_path)
      return None
    return gadget_path

  def SupportsIntelPowerGadget(self):
    gadget_path = self.GetIntelPowerGadgetPath()
    return gadget_path is not None
