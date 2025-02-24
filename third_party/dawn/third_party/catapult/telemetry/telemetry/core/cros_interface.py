# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A wrapper around ssh for common operations on a CrOS-based device"""
from __future__ import absolute_import
import logging
import os
import re
import time

from telemetry.core import linux_based_interface


_CHROME_MOUNT_NAMESPACE_PATH = "/run/namespaces/mnt_chrome"
_IGNORE_FILETYPES_FOR_MINIDUMP_PULLS = (
    linux_based_interface._IGNORE_FILETYPES_FOR_MINIDUMP_PULLS)


DNSFailureException = linux_based_interface.DNSFailureException
KeylessLoginRequiredException = (
    linux_based_interface.KeylessLoginRequiredException)
LoginException = linux_based_interface.LoginException



class CrOSInterface(linux_based_interface.LinuxBasedInterface):

  # Overrides the default path.
  MINIDUMP_DIR = '/var/log/chrome/Crash Reports/'
  _SCREENSHOT_BINARY = '/usr/local/sbin/screenshot'
  _SCREENSHOT_TMP_DIR = '/var/log/screenshots/%s'

  @property
  def display(self):
    # There is no separate set of displays
    return None

  def GetChromeProcess(self):
    """Locates the the main chrome browser process.

    Chrome on cros is usually in /opt/google/chrome, but could be in
    /usr/local/ for developer workflows - debug chrome is too large to fit on
    rootfs.

    Chrome spawns multiple processes for renderers. pids wrap around after they
    are exhausted so looking for the smallest pid is not always correct. We
    locate the session_manager's pid, and look for the chrome process that's an
    immediate child. This is the main browser process.
    """
    procs = self.ListProcesses()
    session_manager_pid = self._GetSessionManagerPid(procs)
    if not session_manager_pid:
      return None

    # Find the chrome process that is the child of the session_manager.
    for pid, process, ppid, _ in procs:
      if ppid != session_manager_pid:
        continue
      for regex in linux_based_interface.CHROME_PROCESS_REGEX:
        path_match = re.match(regex, process)
        if path_match is not None:
          return {'pid': pid, 'path': path_match.group(), 'args': process}
    return None

  def GetChromePid(self):
    """Returns pid of main chrome browser process."""
    result = self.GetChromeProcess()
    if result and 'pid' in result:
      return result['pid']
    return None

  def IsRunningOnVM(self):
    if self._is_running_on_vm is None:
      self._is_running_on_vm = self.RunCmdOnDevice(['crossystem', 'inside_vm'
                                                   ])[0] != '0'
    return self._is_running_on_vm

  def GetDeviceTypeName(self):
    """DEVICETYPE in /etc/lsb-release is CHROMEBOOK, CHROMEBIT, etc."""
    if self._device_type_name is None:
      self._device_type_name = self.LsbReleaseValue(
          key='DEVICETYPE', default='CHROMEBOOK')
    return self._device_type_name

  def GetBoard(self):
    """Gets the name of the board of the device, e.g. "kevin".

    Returns:
      The name of the board as a string, or None if it can't be retrieved.
    """
    if self._board is None:
      self._board = self.LsbReleaseValue(
          key='CHROMEOS_RELEASE_BOARD', default=None)
    return self._board

  def _GetSessionManagerPid(self, procs):
    """Returns the pid of the session_manager process, given the list of
    processes."""
    for pid, process, _, _ in procs:
      argv = process.split()
      if argv and os.path.basename(argv[0]) == 'session_manager':
        return pid
    return None

  def Chown(self, filename, owner='chronos', group='chronos'):
    super().Chown(filename, owner, group)

  def IsServiceRunning(self, service_name):
    """Check with the init daemon if the given service is running."""
    if self.HasSystemd():
      return super().IsServiceRunning(service_name)

    # This fallback exists only for ChromeOS.
    stdout, _ = self.RunCmdOnDevice(['status', service_name], quiet=True)
    running = 'running, process' in stdout
    logging.debug("IsServiceRunning(%s)->%s" % (service_name, running))
    return running

  def _GetMountSourceAndTarget(self, path, ns=None):

    def _RunAndSplit(cmd):
      cmd_out, _ = self.RunCmdOnDevice(cmd)
      return cmd_out.split('\n')

    cmd = ['/bin/df', '--output=source,target', path]
    df_ary = []
    if ns:
      ns_cmd = ['nsenter', '--mount=%s' % ns]
      ns_cmd.extend(cmd)
      # Try running 'df' in the non-root mount namespace.
      df_ary = _RunAndSplit(ns_cmd)

    if len(df_ary) < 3:
      df_ary = _RunAndSplit(cmd)

    # 3 lines for title, mount info, and empty line:
    # # df --output=source,target `cryptohome-path user '$guest'`
    # Filesystem     Mounted on\n
    # /dev/loop6     /home/user/a5715c406109752ce7c31dad219c85c4e812728f\n
    #
    if len(df_ary) == 3:
      line_ary = df_ary[1].split()
      return line_ary if len(line_ary) == 2 else None
    return None

  # Cryptohome related methods are below.
  def FilesystemMountedAt(self, path):
    """Returns the filesystem mounted at |path|"""
    mount_info = self._GetMountSourceAndTarget(path)
    return mount_info[0] if mount_info else None

  def EphemeralCryptohomePath(self, user):
    """Returns the ephemeral cryptohome mount poing for |user|."""
    profile_path = self.CryptohomePath(user)
    # Get user hash as last element of cryptohome path last.
    return os.path.join('/run/cryptohome/ephemeral_mount/',
                        os.path.basename(profile_path))

  def CryptohomePath(self, user):
    """Returns the cryptohome mount point for |user|."""
    stdout, stderr = self.RunCmdOnDevice(
        ['cryptohome-path', 'user', "'%s'" % user])
    if stderr != '':
      raise OSError('cryptohome-path failed: %s' % stderr)
    return stdout.rstrip()

  def IsCryptohomeMounted(self, username, is_guest):
    """Returns True iff |user|'s cryptohome is mounted."""
    # Check whether it's ephemeral mount from a loop device.
    profile_ephemeral_path = self.EphemeralCryptohomePath(username)
    ns = None
    if is_guest:
      ns = _CHROME_MOUNT_NAMESPACE_PATH
    ephemeral_mount_info = self._GetMountSourceAndTarget(
        profile_ephemeral_path, ns)
    if ephemeral_mount_info:
      return (ephemeral_mount_info[0].startswith('/dev/loop')
              and ephemeral_mount_info[1] == profile_ephemeral_path)

    profile_path = self.CryptohomePath(username)
    mount_info = self._GetMountSourceAndTarget(profile_path)
    if mount_info:
      # Checks if the filesytem at |profile_path| is mounted on |profile_path|
      # itself. Before mounting cryptohome, it shows an upper directory (/home).
      is_guestfs = (mount_info[0] == 'guestfs')
      return is_guestfs == is_guest and mount_info[1] == profile_path
    return False

  def StopUI(self):
    stop_cmd = ['stop', 'ui']
    if self.HasSystemd():
      stop_cmd.insert(0, 'systemctl')
    self.RunCmdOnDevice(stop_cmd)

  def RestartUI(self):
    logging.info('(Re)starting the ui (logs the user out)')
    start_cmd = ['start', 'ui']
    restart_cmd = ['restart', 'ui']
    if self.HasSystemd():
      start_cmd.insert(0, 'systemctl')
      restart_cmd.insert(0, 'systemctl')

    if self.IsServiceRunning('ui'):
      self.RunCmdOnDevice(restart_cmd)
    else:
      self.RunCmdOnDevice(start_cmd)

  def _DisableRootFsVerification(self):
    """Disables rootfs verification on the device, requiring a reboot."""
    # 2 and 4 are the kernel partitions.
    for partition in [2, 4]:
      self.RunCmdOnDevice([
          '/usr/share/vboot/bin/make_dev_ssd.sh', '--partitions',
          str(partition), '--remove_rootfs_verification', '--force'
      ])

    # Restart, wait a bit, and re-establish the SSH master connection.
    # We need to close the connection gracefully, then run the shutdown command
    # without using a master connection. port_forward=True bypasses the master
    # connection.
    self.CloseConnection()
    self.RunCmdOnDevice(['reboot'], port_forward=True)
    time.sleep(30)
    self.OpenConnection()

  def _RemountRootAsReadWrite(self):
    """Remounts / as a read-write partition."""
    self.RunCmdOnDevice(['mount', '-o', 'remount,rw', '/'])

  def _ScreenshotCmd(self, file_path):
    return [self._SCREENSHOT_BINARY, file_path]
