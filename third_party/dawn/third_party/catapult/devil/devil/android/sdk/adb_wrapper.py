# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This module wraps Android's adb tool.

This is a thin wrapper around the adb interface. Any additional complexity
should be delegated to a higher level (ex. DeviceUtils).
"""

from __future__ import absolute_import
import collections
# pylint: disable=import-error
# pylint: disable=no-name-in-module
from datetime import datetime, timedelta
import distutils.version as du_version
import errno
import logging
import os
import posixpath
import re
import subprocess
import threading
import time
from six.moves.queue import Queue, Empty

import six

from devil import base_error
from devil import devil_env
from devil.android import decorators
from devil.android import device_errors
from devil.android.sdk import version_codes
from devil.utils import cmd_helper
from devil.utils import lazy
from devil.utils import timeout_retry

with devil_env.SysPath(devil_env.DEPENDENCY_MANAGER_PATH):
  import dependency_manager  # pylint: disable=import-error

logger = logging.getLogger(__name__)

ADB_KEYS_FILE = '/data/misc/adb/adb_keys'
ADB_HOST_KEYS_DIR = os.path.join(os.path.expanduser('~'), '.android')

DEFAULT_TIMEOUT = 30
DEFAULT_LONG_TIMEOUT = DEFAULT_TIMEOUT * 10
DEFAULT_SUPER_LONG_TIMEOUT = DEFAULT_LONG_TIMEOUT * 2
DEFAULT_RETRIES = 2

_ADB_PROTOCOL_VERSION_RE = re.compile(
    r'Android Debug Bridge version (\d+\.\d+\.\d+)')
_ADB_RELEASE_VERSION_RE = re.compile(r'Version (\d+\.\d+\.\d+)')
_EMULATOR_RE = re.compile(r'^emulator-[0-9]+$')
_DEVICE_NOT_FOUND_RE = re.compile(r"device '(?P<serial>.+)' not found")
_READY_STATE = 'device'
_VERITY_DISABLE_RE = re.compile(r'(V|v)erity (is )?(already )?disabled'
                                r'|Successfully disabled verity')
_VERITY_ENABLE_RE = re.compile(r'(V|v)erity (is )?(already )?enabled'
                               r'|Successfully enabled verity')
_WAITING_FOR_DEVICE_RE = re.compile(r'- waiting for device -')


def VerifyLocalFileExists(path):
  """Verifies a local file exists.

  Args:
    path: Path to the local file.

  Raises:
    IOError: If the file doesn't exist.
  """
  if not os.path.exists(path):
    raise IOError(errno.ENOENT, os.strerror(errno.ENOENT), path)


def _CreateAdbEnvironment():
  adb_env = dict(os.environ)
  adb_env['ADB_LIBUSB'] = '0'
  return adb_env


def _FindAdb():
  try:
    return devil_env.config.LocalPath('adb')
  except dependency_manager.NoPathFoundError:
    pass

  try:
    return os.path.join(
        devil_env.config.LocalPath('android_sdk'), 'platform-tools', 'adb')
  except dependency_manager.NoPathFoundError:
    pass

  try:
    return devil_env.config.FetchPath('adb')
  except dependency_manager.NoPathFoundError:
    raise device_errors.NoAdbError()


def _GetProtocolVersion():
  # pylint: disable=protected-access
  raw_version = AdbWrapper._RunAdbCmd(['version'], timeout=2, retries=0)
  for l in raw_version.splitlines():
    m = _ADB_PROTOCOL_VERSION_RE.search(l)
    if m:
      return m.group(1)
  return None


def _GetReleaseVersion():
  # pylint: disable=protected-access
  raw_version = AdbWrapper._RunAdbCmd(['version'], timeout=2, retries=0)
  for l in raw_version.splitlines():
    m = _ADB_RELEASE_VERSION_RE.search(l)
    if m:
      return m.group(1)
  return None


def ShouldRetryAfterAdbServerRestart(exc):
  try:
    if isinstance(exc, device_errors.CommandTimeoutError):
      logger.info('Restarting the adb server')
      RestartServer()
    return True
  except Exception:  # pylint: disable=broad-except
    logger.exception(('Caught an exception when deciding'
                      ' to retry a function'))
    return False


def _ShouldRetryAdbCmd(exc):
  # Errors are potentially transient and should be retried, with the exception
  # of NoAdbError. Exceptions [e.g. generated from SIGTERM handler] should be
  # raised.
  return (isinstance(exc, base_error.BaseError)
          and not isinstance(exc, device_errors.NoAdbError))


DeviceStat = collections.namedtuple('DeviceStat',
                                    ['st_mode', 'st_size', 'st_time'])


def _IsExtraneousLine(line, send_cmd):
  """Determine if a line read from stdout in persistent shell is extraneous.

  The results output to stdout by the persistent shell process
  (in PersistentShell below) often include "extraneous" lines that are
  not part of the output of the shell command. These "extraneous" lines
  do not always appear and are of two forms: shell prompt lines and lines
  that just duplicate what the input command was. This function
  detects these extraneous lines. Since all these lines have the
  original command in them, that is what it detects ror.

  Args:
      line: Output line to check.
      send_cmd: Command that was sent to adb persistent shell.
  """
  return send_cmd.rstrip() in line


@decorators.WithExplicitTimeoutAndRetries(timeout=60, retries=3)
def RestartServer():
  """Restarts the adb server.

  Raises:
    CommandFailedError if we fail to kill or restart the server.
  """

  def adb_killed():
    return not AdbWrapper.IsServerOnline()

  def adb_started():
    return AdbWrapper.IsServerOnline()

  AdbWrapper.KillServer()
  if not timeout_retry.WaitFor(adb_killed, wait_period=1, max_tries=5):
    # TODO(crbug.com/442319): Switch this to raise an exception if we
    # figure out why sometimes not all adb servers on bots get killed.
    logger.warning('Failed to kill adb server')
  AdbWrapper.StartServer()
  if not timeout_retry.WaitFor(adb_started, wait_period=1, max_tries=5):
    raise device_errors.CommandFailedError('Failed to start adb server')


class AdbWrapper(object):
  """A wrapper around a local Android Debug Bridge executable."""

  _ADB_ENV = _CreateAdbEnvironment()

  _adb_path = lazy.WeakConstant(_FindAdb)
  _adb_protocol_version = lazy.WeakConstant(_GetProtocolVersion)
  _adb_release_version = lazy.WeakConstant(_GetReleaseVersion)

  class PersistentShell(object):
    '''Class to use persistent shell for ADB.

    This class allows a persistent ADB shell to be created, where multiple
    commands can be passed into it. This avoids the overhead of starting
    up a new ADB shell for each command.

    A PersistentShell is composed of a subprocess running an "adb shell"
    command, with a thread grabbing the output.

    Example of use:
    with PersistentShell('123456789') as pshell:
        pshell.RunCommand('which ls')
        pshell.RunCommand('echo TEST', close=True)
    '''
    # These need to be short to keep runtime lower.
    _SHELL_OUTPUT_START_MARKER = 'StArT_M'
    _SHELL_OUTPUT_END_MARKER = 'DoNe_M'

    def __init__(self, serial, registry=None):
      """Initialization function:

      Args:
        serial: Serial number of device.
        registry: An AdbWrapper object whose queue the persistent shell should
            be added to when done. Not used for context-manager persistent
            shell calls or if re-use of the shell isn't desired.
      """
      self._cmd = [AdbWrapper.GetAdbPath(), '-s', serial, 'shell']
      self._process = None
      self._device_serial = serial
      self._start_found = False
      self._outq = None
      self._reader_thread = None
      self._terminating = False
      self._registry = registry

    def __enter__(self):
      self.Start()
      self.WaitForReady()
      return self

    def __exit__(self, exc_type, exc_value, tb):
      self.Stop()

    def RunCommand(self, command, close=False, keepends=False):
      """Runs an ADB command and returns the output.

      Args:
        command: Command to send.
        close: If true, terminates the persistent shell after the command.
        keepends: If False, output lines have trailing whitespace removed.

      Rasies:
        AdbShellCommandFailedError - if the persistent shell crashes.

      Returns:
        ([outline1, outline2], 0) - Array of output lines and the status code
      """
      if self._process is None:
        raise device_errors.CommandFailedError('No adb shell process found.')

      output_iter = self.IterRunCommand(command, True, close)
      result = []
      for line in output_iter:
        # Persistent shell forces a newline to be at the end of a command. But
        # if that command already has a newline, then the output will have an
        # extra blank newline, so filter it out.
        if isinstance(line, str):
          # Sometimes the newline is "\n" and sometimes it's "\r\n" depending
          # on certain devices.
          if line.strip() == '':
            continue
          if not keepends:
            line = line.rstrip()

        result.append(line)

      return (result[:-1], result[-1])

    def Stop(self):
      """Stops the ADB process if it is still running."""
      # If command is called with closed, then process may be 0.
      if self._process is not None and self._process.poll() != 0:
        self._process.stdin.write(six.ensure_binary('exit\n'))
        self._process.stdin.flush()
        self._process = None

      self._reader_thread = None
      self._outq = None

    def EnsureStarted(self, force_restart=False):
      """Ensures the shell is running and ready for commands.

      Will restart the shell in case it has crashed.
      """
      if self._process is not None:
        if force_restart:
          self._process.kill()
          retcode = None
        else:
          retcode = self._process.poll()
          # If no return code, shell process is alive and hopefully well.
          if retcode is None:
            return
        logging.warning("Adb PersistentShell crashed with code %d", retcode)
        self._process = None

      # self._process will always be None at this point.
      self.Start()

    def HardStop(self):
      """Kills the adb process immediately.

      Immediately kills the adb process and the persistent shell will be
      removed from the queue of persistent shells.
      """
      self._terminating = True
      self._reader_thread = None
      self._outq = None
      if self._process is not None and self._process.poll() is None:
        self._process.kill()

    def Start(self):
      """Start the shell."""
      if self._process is not None:
        raise RuntimeError('Persistent shell already running.')
      # pylint: disable=protected-access
      self._process = subprocess.Popen(self._cmd,
                                       stdin=subprocess.PIPE,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.STDOUT,
                                       shell=False,
                                       env=AdbWrapper._ADB_ENV)

      def output_reader(proc, outq):
        for line in iter(proc.stdout.readline, b''):
          outq.put(line)

      self._outq = Queue()
      self._reader_thread = threading.Thread(target=output_reader,
                                             args=(self._process, self._outq))
      self._reader_thread.setDaemon(True)
      self._reader_thread.start()

    def WaitForReady(self):
      """Wait for the shell to be ready after starting.

      Sends an echo command, and raises an exception if it gets a bad status.
      Raises:
        AdbShellCommandFailedError
      """
      cmd = 'echo\n'
      _, status = self.RunCommand(cmd)
      if status:
        raise device_errors.AdbShellCommandFailedError(
            cmd,
            'Failed to get ready status from adb shell.',
            status=status,
            device_serial=self._device_serial)

    def IterRunCommand(
        self,
        command,
        include_status,
        close=False,
    ):
      """Runs an ADB command and yields the output line by line.

      Args:
        command: Command to send.
        include_status: When False, the status code is not included in output.
            When True, the status code, as an int, is the last item.
        close: If true, terminates the persistent shell after the command.

      Rasies:
        AdbShellCommandFailedError - if the persistent shell crashes.

      Yields:
        The output of the command line by line. If include_status is True, the
        last generated output is an integer status code.
      """
      if close:
        send_cmd = '( %s ); echo %s$?; exit;\n' % (
            command, self._SHELL_OUTPUT_END_MARKER)
        # This could be simplified with a communicate call, but py2 throws
        # an error from the thread reader and communicate accessing the same
        # fd.
        self._process.stdin.write(six.ensure_binary(send_cmd))
        self._process.stdin.flush()
        output_lines = []
        while True:
          try:
            self._process.stdout.flush()
            line = six.ensure_str(self._outq.get(timeout=DEFAULT_TIMEOUT))
            if not _IsExtraneousLine(line, send_cmd):
              # This allows us to check for the exit code to know when to stop
              # looking for output.
              if line.startswith(self._SHELL_OUTPUT_END_MARKER):
                exit_code = int(line[len(self._SHELL_OUTPUT_END_MARKER):])
                break
              output_lines.append(line)
          except Empty:
            break

        # Wait for process finish.
        # This could be simplified with |self._process.wait(5)| after
        # migration to py3-only.
        timeout_if_no_finish = datetime.now() + timedelta(seconds=8)
        while self._process.poll() is None:
          if datetime.now() > timeout_if_no_finish:
            logging.warning('Adb wedged. Kill')
            self._process.kill()
            break
          time.sleep(0.1)

        self._terminating = True
        if include_status:
          output_lines.append(exit_code)

        for x in output_lines:
          yield x

      else:
        # Ensure there's a newline before DONE as not all commands send a
        # newline.
        send_cmd = 'echo %s; ( %s ); echo \\\\n%s$?\n' % (
            self._SHELL_OUTPUT_START_MARKER, command,
            self._SHELL_OUTPUT_END_MARKER)
        if self._start_found:
          # A lock has been busted midway through a command to the other
          # process. Hard restart the shell.
          self.EnsureStarted(force_restart=True)

        self._process.stdin.write(six.ensure_binary(send_cmd))
        self._process.stdin.flush()  # Ensure underlying stdio flushes.
        timeout_if_no_start_by = datetime.now() + timedelta(milliseconds=4000)

        # Enter select loop for subprocess to avoid deadlock problems
        # when reading/writing to children via pipes.
        while True:
          if not self._start_found and datetime.now() > timeout_if_no_start_by:
            logging.warning("Adb wedged. Restarting.")
            # Process is wedged. Restart it.
            if self._process.poll() is None:
              self._process.kill()

            self._process = None
            self.Start()
            self._process.stdin.write(send_cmd.encode('utf-8'))
            self._process.stdin.flush()  # Ensure underlying stdio flushes.
            timeout_if_no_start_by = datetime.now() + timedelta(
                milliseconds=2000)

          if self._process.poll() is not None:
            raise device_errors.AdbShellCommandFailedError(
                command,
                "ADB shell crashed",
                status=None,
                device_serial=self._device_serial)

          # Read pipe extracting between StArT and DoNe
          try:
            output_line = six.ensure_str(self._outq.get(timeout=0.1))
          except Empty:
            continue

          # On some versions of android (observed on marshmallow bot), adb
          # shell has a bug where the first line returned after a write
          # seems to be a corrupted buffer that often, but not always,
          # contains the shell prompt plus command written. Inserting an
          # explicit output start marker allows us to skip over these bad
          # lines.
          if not self._start_found:
            if output_line.startswith(self._SHELL_OUTPUT_START_MARKER):
              self._start_found = True
            continue

          if output_line.startswith(self._SHELL_OUTPUT_END_MARKER):
            self._start_found = False
            if include_status:
              yield int(output_line[len(self._SHELL_OUTPUT_END_MARKER):])
            break
          yield output_line

      if self._registry:
        if self._terminating:
          self._registry.KillPersistentShell(self)
        else:
          self._registry.ReAddAdb(self)

  def __init__(self, device_serial, persistent_shell=False):
    """Initializes the AdbWrapper.

    Args:
      device_serial: The device serial number as a string.
      persistent_shell: If True, uses a long-running "adb shell" for executing
          commands
    """
    if not device_serial:
      raise ValueError('A device serial must be specified')
    self._device_serial = str(device_serial)

    # A queue of persistent shells that are waiting for a command to execute.
    # When a persistent shell is done executing a command, it is usually put
    # back into the queue to await the next command.
    self._idle_persistent_shells_queue = None
    # A list of all persistent shells processes that the adb instance owns.
    # This includes shells that are currenly running a command and those that
    # are in the queue waiting for a command to run.
    self._all_persistent_shells = []
    self._lock = threading.Lock()

    # The test blinkpy.web_tests.port.android_unittest.AndroidPortTest.
    # test_default_child_processes doesn't work with checking for a
    # real device on some builders.
    if not persistent_shell:
      return

    for _ in range(5):
      if not self.is_ready:
        # Sometimes emulators that are being started up will indicate they're
        # available to 'adb devices' but will not be ready to connect on adb.
        time.sleep(3)
        break

    if persistent_shell:
      if not self.is_ready:
        raise device_errors.DeviceUnreachableError(self._device_serial)
      self._idle_persistent_shells_queue = Queue()
      # Use lock to prevent shells from being killed while we're adding.
      self._lock.acquire()
      # TODO: Add multiple shells for speed improvement. This makes sure that
      # there is already a shell in the queue waiting if another adb command
      # comes in while the first shell is being used.
      shell = self.PersistentShell(self._device_serial, registry=self)
      shell.Start()
      self._idle_persistent_shells_queue.put(shell)
      self._all_persistent_shells.append(shell)
      self._lock.release()

  def ReAddAdb(self, adb):
    """Puts the adb back in the persistent shell queue.

    It is desirable to keep the adbs in the queue for faster runtime, avoiding
    the need to start up a new shell.
    """
    if self._idle_persistent_shells_queue:
      self._idle_persistent_shells_queue.put(adb)

  def KillPersistentShell(self, adb):
    """Kills the persistent shell.

    Args:
      adb: The adbWrapper that is running a persistent shell.
    """
    self._all_persistent_shells.remove(adb)
    adb.HardStop()

  def KillAllPersistentAdbs(self):
    """Kills all the persistent shells that the adb wrapper is running.

    """
    self._lock.acquire()
    aps = self._all_persistent_shells
    self._all_persistent_shells = []

    for x in aps:
      x.HardStop()

    if self._idle_persistent_shells_queue:
      try:
        while True:
          self._idle_persistent_shells_queue.get(block=False)
      except Empty:
        pass
    self._lock.release()

  @classmethod
  def GetAdbPath(cls):
    return cls._adb_path.read()

  @classmethod
  def Version(cls):
    """Returns the adb "protocol version number" (obsolete).

    This is referred to by the adb team as the "protocol version number" and is
    no longer being incremented for new features due to it being too disruptive
    and forcing a server restart.

    Prefer to call ReleaseVersion instead. Context:
    http://issuetracker.google.com/218716282#comment8
    """
    return cls._adb_protocol_version.read()

  @classmethod
  def ReleaseVersion(cls):
    """Returns the adb "release version number".

    Prefer to use this to gate features and fixes as this is the platform-tools
    version and it is incremented with each release. Context:
    http://issuetracker.google.com/218716282#comment8
    """
    return cls._adb_release_version.read()

  @classmethod
  def _BuildAdbCmd(cls, args, device_serial, cpu_affinity=None):
    if cpu_affinity is not None:
      cmd = ['taskset', '-c', str(cpu_affinity)]
    else:
      cmd = []
    cmd.append(cls.GetAdbPath())
    if device_serial is not None:
      cmd.extend(['-s', device_serial])
    cmd.extend(args)
    return cmd

  # pylint: disable=unused-argument
  @classmethod
  @decorators.WithTimeoutAndConditionalRetries(_ShouldRetryAdbCmd)
  def _RunAdbCmd(cls,
                 args,
                 timeout=None,
                 retries=None,
                 device_serial=None,
                 check_error=True,
                 cpu_affinity=None,
                 additional_env=None):
    if timeout:
      remaining = timeout_retry.CurrentTimeoutThreadGroup().GetRemainingTime()
      if remaining:
        # Use a slightly smaller timeout than remaining time to ensure that we
        # have time to collect output from the command.
        cmd_timeout = 0.95 * remaining
      else:
        cmd_timeout = None
    else:
      cmd_timeout = timeout
    env = cls._ADB_ENV.copy()
    if additional_env:
      env.update(additional_env)
    adb_cmd = cls._BuildAdbCmd(args, device_serial, cpu_affinity=cpu_affinity)
    try:
      status, output = cmd_helper.GetCmdStatusAndOutputWithTimeout(adb_cmd,
                                                                   cmd_timeout,
                                                                   env=env)
    except OSError as e:
      if e.errno in (errno.ENOENT, errno.ENOEXEC):
        raise device_errors.NoAdbError(msg=str(e))
      raise
    except cmd_helper.TimeoutError:
      logger.exception(
          'Timeout on adb command (timeout=%r, retries=%r, '
          'cmd_timeout=%r): %r', timeout, retries, cmd_timeout, adb_cmd)
      raise

    # Best effort to catch errors from adb; unfortunately adb is very
    # inconsistent with error reporting so many command failures present
    # differently.
    if check_error and (status != 0 or output.startswith('error:')):
      not_found_m = _DEVICE_NOT_FOUND_RE.search(output)
      device_waiting_m = _WAITING_FOR_DEVICE_RE.match(output)
      if (device_waiting_m is not None
          or (not_found_m is not None
              and not_found_m.group('serial') == device_serial)):
        raise device_errors.DeviceUnreachableError(device_serial)
      raise device_errors.AdbCommandFailedError(args, output, status,
                                                device_serial)

    return output

  # pylint: enable=unused-argument

  def _RunDeviceAdbCmd(self, args, timeout, retries, check_error=True):
    """Runs an adb command on the device associated with this object.

    Args:
      args: A list of arguments to adb.
      timeout: Timeout in seconds.
      retries: Number of retries.
      check_error: Check that the command doesn't return an error message. This
        checks both the error status of adb and the exit status of shell
        commands.

    Returns:
      The output of the command.
    """
    return self._RunAdbCmd(
        args,
        timeout=timeout,
        retries=retries,
        device_serial=self._device_serial,
        check_error=check_error)

  def _IterRunDeviceAdbCmd(self, args, iter_timeout, timeout, check_error=True):
    """Runs an adb command and returns an iterator over its output lines.

    Args:
      args: A list of arguments to adb.
      iter_timeout: Timeout for each iteration in seconds.
      timeout: Timeout for the entire command in seconds.
      check_error: Check that the command succeeded. This does check the
        error status of the adb command but DOES NOT check the exit status
        of shell commands.

    Yields:
      The output of the command line by line.
    """
    return cmd_helper.IterCmdOutputLines(
        self._BuildAdbCmd(args, self._device_serial),
        iter_timeout=iter_timeout,
        timeout=timeout,
        env=self._ADB_ENV,
        check_status=check_error)

  def _Shell(self,
             command,
             expect_status=None,
             timeout=DEFAULT_TIMEOUT,
             retries=DEFAULT_RETRIES):
    """Runs a shell command on the device.

    Args:
      command: A string with the shell command to run.
      expect_status: (optional) If None, skip status check.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.

    Returns:
      A tuple: the first value is the output of the shell command as a string;
      the second value is the integer status code of the command if
      expect_status=True, otherwise this is None.
    Raises:
      DeviceUnreachableError on missing device.
    """
    # Pipe stderr->stdout to ensure the echo'ed exit code is not interleaved
    # with the command's stderr (as seen in https://crbug.com/1314912).
    args = ['shell', '( %s ) 2>&1;echo %%$?' % command.rstrip()]
    output = self._RunDeviceAdbCmd(args, timeout, retries)
    # If we don't care about the status, just return output and unchecked
    # status.
    if expect_status is None:
      return (output, None)

    output_end = output.rfind('%')
    # We care about the status, and the and exit status is not found.
    if output_end < 0:
      logger.warning('exit status of shell command %r missing.', command)
      return (output, None)

    # There could be a case where we don't get a return status, but the error
    # message contains a '%' symbol.
    try:
      status = int(output[output_end + 1:])
    except ValueError:
      logger.warning('exit status of shell command %r missing.', command)
      return (output, None)

    return (output[:output_end], status)

  def __eq__(self, other):
    """Consider instances equal if they refer to the same device.

    Args:
      other: The instance to compare equality with.

    Returns:
      True if the instances are considered equal, false otherwise.
    """
    return self._device_serial == str(other)

  def __str__(self):
    """The string representation of an instance.

    Returns:
      The device serial number as a string.
    """
    return self._device_serial

  def __repr__(self):
    return '%s(\'%s\')' % (self.__class__.__name__, self)

  # pylint: disable=unused-argument
  @classmethod
  def IsServerOnline(cls):
    status, output = cmd_helper.GetCmdStatusAndOutput(['pgrep', 'adb'])
    output = [int(x) for x in output.split()]
    logger.info('PIDs for adb found: %r', output)
    return status == 0

  # pylint: enable=unused-argument

  @classmethod
  def KillServer(cls, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    cls._RunAdbCmd(['kill-server'], timeout=timeout, retries=retries)

  @classmethod
  def StartServer(cls,
                  keys=None,
                  timeout=DEFAULT_TIMEOUT,
                  retries=DEFAULT_RETRIES):
    """Starts the ADB server.

    Args:
      keys: (optional) List of local ADB keys to use to auth with devices.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    """
    additional_env = {}
    if keys:
      additional_env['ADB_VENDOR_KEYS'] = ':'.join(keys)
    # CPU affinity is used to reduce adb instability http://crbug.com/268450
    cls._RunAdbCmd(['start-server'],
                   timeout=timeout,
                   retries=retries,
                   cpu_affinity=0,
                   additional_env=additional_env)

  @classmethod
  def GetDevices(cls, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """DEPRECATED. Refer to Devices(...) below."""
    # TODO(jbudorick): Remove this function once no more clients are using it.
    return cls.Devices(timeout=timeout, retries=retries)

  @classmethod
  def Devices(cls,
              desired_state=_READY_STATE,
              long_list=False,
              persistent_shell=False,
              timeout=DEFAULT_TIMEOUT,
              retries=DEFAULT_RETRIES):
    """Get the list of active attached devices.

    Args:
      desired_state: If not None, limit the devices returned to only those
        in the given state.
      long_list: Whether to use the long listing format.
      persistent_shell: If True, uses a single "adb shell" for running commands.
      timeout: Timeout per try in seconds.
      retries: Number of retries to attempt.

    Yields:
      AdbWrapper instances.
    """
    lines = cls._RawDevices(
        long_list=long_list, timeout=timeout, retries=retries)
    if long_list:
      return [
          [AdbWrapper(line[0], persistent_shell=persistent_shell)] + line[1:]
          for line in lines
          if (len(line) >= 2 and (not desired_state or line[1] == desired_state)
              )
      ]

    return [
        AdbWrapper(line[0], persistent_shell=persistent_shell) for line in lines
        if (len(line) == 2 and (not desired_state or line[1] == desired_state))
    ]

  @classmethod
  def _RawDevices(cls,
                  long_list=False,
                  timeout=DEFAULT_TIMEOUT,
                  retries=DEFAULT_RETRIES):
    cmd = ['devices']
    if long_list:
      cmd.append('-l')
    output = cls._RunAdbCmd(cmd, timeout=timeout, retries=retries)
    return [line.split() for line in output.splitlines()[1:]]

  def _CheckSdkVersion(self, expected_sdk, error_message):
    """Checks the SDK version reported by the device and throws an error if
    it is too low

    Throws: device_errors.DeviceVersionError
    """
    raw_version = self._RunDeviceAdbCmd(
        ['shell', 'getprop', 'ro.build.version.sdk'], timeout=2, retries=0)
    device_version = int(raw_version.rstrip())
    if device_version < expected_sdk:
      raise device_errors.DeviceVersionError(
          '%s: SDK version %d expected but %d reported' %
          (error_message, expected_sdk, device_version))

  def GetDeviceSerial(self):
    """Gets the device serial number associated with this object.

    Returns:
      Device serial number as a string.
    """
    return self._device_serial

  def Push(self,
           local,
           remote,
           sync=False,
           timeout=DEFAULT_SUPER_LONG_TIMEOUT,
           retries=DEFAULT_RETRIES):
    """Pushes a file from the host to the device.

    Args:
      local: Path on the host filesystem.
      remote: Path on the device filesystem.
      sync: (optional) Whether to only push files that are newer on the host.
        Not supported when using adb prior to 1.0.39.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.

    Raises:
      AdbVersionError if sync=True with versions of adb prior to 1.0.39.
    """
    VerifyLocalFileExists(local)

    if (du_version.LooseVersion(self.Version()) <
        du_version.LooseVersion('1.0.36')):

      # Different versions of adb handle pushing a directory to an existing
      # directory differently.

      # In the version packaged with the M SDK, 1.0.32, the following push:
      #   foo/bar -> /sdcard/foo/bar
      # where bar is an existing directory both on the host and the device
      # results in the contents of bar/ on the host being pushed to bar/ on
      # the device, i.e.
      #   foo/bar/A -> /sdcard/foo/bar/A
      #   foo/bar/B -> /sdcard/foo/bar/B
      #   ... etc.

      # In the version packaged with the N SDK, 1.0.36, the same push under
      # the same conditions results in a second bar/ directory being created
      # underneath the first bar/ directory on the device, i.e.
      #   foo/bar/A -> /sdcard/foo/bar/bar/A
      #   foo/bar/B -> /sdcard/foo/bar/bar/B
      #   ... etc.

      # In order to provide a consistent interface to clients, we check whether
      # the target is an existing directory on the device and, if so, modifies
      # the target passed to adb to emulate the behavior on 1.0.36 and above.

      # Note that this behavior may have started before 1.0.36; that's simply
      # the earliest version we've confirmed thus far.

      try:
        self.Shell('test -d %s' % remote, timeout=timeout, retries=retries)
        remote = posixpath.join(remote, posixpath.basename(local))
      except device_errors.AdbShellCommandFailedError:
        # The target directory doesn't exist on the device, so we can use it
        # without modification.
        pass

    push_cmd = ['push']

    if sync:
      push_cmd += ['--sync']
      if (du_version.LooseVersion(self.Version()) <
          du_version.LooseVersion('1.0.39')):
        # The --sync flag for `adb push` is a relatively recent addition.
        # We're not sure exactly which release first contained it, but it
        # exists at least as far back as 1.0.39.
        raise device_errors.AdbVersionError(
            push_cmd,
            desc='--sync not supported',
            actual_version=self.Version(),
            min_version='1.0.39')

    push_cmd += [local, remote]

    self._RunDeviceAdbCmd(push_cmd, timeout, retries)

  def Pull(self,
           remote,
           local,
           timeout=DEFAULT_SUPER_LONG_TIMEOUT,
           retries=DEFAULT_RETRIES):
    """Pulls a file or directory from the device to the host.

    Args:
      remote: Path on the device filesystem.
      local: Path on the host filesystem.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    """
    cmd = ['pull', remote, local]
    self._RunDeviceAdbCmd(cmd, timeout, retries)
    try:
      VerifyLocalFileExists(local)
    except IOError:
      raise device_errors.AdbCommandFailedError(
          cmd,
          'File pulled from the device did not arrive on the host: %s' % local,
          device_serial=str(self))

  def StartShell(self, cmd):
    """Starts a subprocess on the device and returns a handle to the process.

    Args:
      args: A sequence of program arguments. The executable to run is the first
        item in the sequence.

    Returns:
      An instance of subprocess.Popen associated with the live process.
    """
    return cmd_helper.StartCmd(
        self._BuildAdbCmd(['shell'] + cmd, self._device_serial))

  def Shell(self,
            command,
            expect_status=0,
            timeout=DEFAULT_TIMEOUT,
            retries=DEFAULT_RETRIES):
    """Runs a shell command on the device.

    Args:
      command: A string with the shell command to run.
      expect_status: (optional) Check that the command's exit status matches
        this value. Default is 0. If set to None the test is skipped.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.

    Returns:
      The output of the shell command as a string.

    Raises:
      device_errors.AdbCommandFailedError: If the exit status doesn't match
        |expect_status|.
    """
    if self._idle_persistent_shells_queue:
      output, status = self._get_a_shell().RunCommand(command, keepends=True)
      output = ''.join(output).rstrip()
    else:
      output, status = self._Shell(command, expect_status, timeout, retries)

    if expect_status is not None and status != expect_status:
      raise device_errors.AdbShellCommandFailedError(
          command, output, status=status, device_serial=self._device_serial)

    return output

  def IterShell(self, command, timeout):
    """Runs a shell command and returns an iterator over its output lines.

    The output lines are returned while the command is still running.

    Args:
      command: A string with the shell command to run.
      timeout: Timeout in seconds.

    Yields:
      The output of the command line by line.
    """
    if self._idle_persistent_shells_queue:
      return self._get_a_shell().IterRunCommand(command, False)

    args = ['shell', command]
    return cmd_helper.IterCmdOutputLines(self._BuildAdbCmd(
        args, self._device_serial),
                                         timeout=timeout,
                                         env=self._ADB_ENV)

  def Ls(self, path, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """List the contents of a directory on the device.

    Args:
      path: Path on the device filesystem.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.

    Returns:
      A list of pairs (filename, stat) for each file found in the directory,
      where the stat object has the properties: st_mode, st_size, and st_time.

    Raises:
      AdbCommandFailedError if |path| does not specify a valid and accessible
          directory in the device, or the output of "adb ls" command is less
          than four columns
    """

    def ParseLine(line, cmd):
      cols = line.split(None, 3)
      if len(cols) < 4:
        raise device_errors.AdbCommandFailedError(
            cmd,
            line,
            "the output should be 4 columns, but is only %d columns" %
            len(cols),
            device_serial=self._device_serial)
      filename = cols.pop()
      stat = DeviceStat(*[int(num, base=16) for num in cols])
      return (filename, stat)

    cmd = ['ls', path]
    lines = self._RunDeviceAdbCmd(
        cmd, timeout=timeout, retries=retries).splitlines()
    if lines:
      return [ParseLine(line, cmd) for line in lines]
    raise device_errors.AdbCommandFailedError(
        cmd,
        'path does not specify an accessible directory in the device',
        device_serial=self._device_serial)

  def Logcat(self,
             clear=False,
             dump=False,
             filter_specs=None,
             logcat_format=None,
             ring_buffer=None,
             iter_timeout=None,
             check_error=True,
             timeout=None,
             retries=DEFAULT_RETRIES):
    """Get an iterable over the logcat output.

    Args:
      clear: If true, clear the logcat.
      dump: If true, dump the current logcat contents.
      filter_specs: If set, a list of specs to filter the logcat.
      logcat_format: If set, the format in which the logcat should be output.
        Options include "brief", "process", "tag", "thread", "raw", "time",
        "threadtime", and "long"
      ring_buffer: If set, a list of alternate ring buffers to request.
        Options include "main", "system", "radio", "events", "crash" or "all".
        The default is equivalent to ["main", "system", "crash"].
      iter_timeout: If set and neither clear nor dump is set, the number of
        seconds to wait between iterations. If no line is found before the
        given number of seconds elapses, the iterable will yield None.
      check_error: Whether to check the exit status of the logcat command.
      timeout: (optional) If set, timeout per try in seconds. If clear or dump
        is set, defaults to DEFAULT_TIMEOUT.
      retries: (optional) If clear or dump is set, the number of retries to
        attempt. Otherwise, does nothing.

    Yields:
      logcat output line by line.
    """
    cmd = ['logcat']
    use_iter = True
    if clear:
      cmd.append('-c')
      use_iter = False
    if dump:
      cmd.append('-d')
      use_iter = False
    if logcat_format:
      cmd.extend(['-v', logcat_format])
    if ring_buffer:
      for buffer_name in ring_buffer:
        cmd.extend(['-b', buffer_name])
    if filter_specs:
      cmd.extend(filter_specs)

    if use_iter:
      return self._IterRunDeviceAdbCmd(
          cmd, iter_timeout, timeout, check_error=check_error)
    timeout = timeout if timeout is not None else DEFAULT_TIMEOUT
    output = self._RunDeviceAdbCmd(cmd,
                                   timeout,
                                   retries,
                                   check_error=check_error)
    return output.splitlines()

  def Forward(self,
              local,
              remote,
              allow_rebind=False,
              timeout=DEFAULT_TIMEOUT,
              retries=DEFAULT_RETRIES):
    """Forward socket connections from the local socket to the remote socket.

    Sockets are specified by one of:
      tcp:<port>
      localabstract:<unix domain socket name>
      localreserved:<unix domain socket name>
      localfilesystem:<unix domain socket name>
      dev:<character device name>
      jdwp:<process pid> (remote only)

    Args:
      local: The host socket.
      remote: The device socket.
      allow_rebind: A boolean indicating whether adb may rebind a local socket;
        otherwise, the default, an exception is raised if the local socket is
        already being forwarded.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    Returns:
      The output of the "adb forward" command as a string.
    """
    cmd = ['forward']
    if not allow_rebind:
      cmd.append('--no-rebind')
    cmd.extend([str(local), str(remote)])
    return self._RunDeviceAdbCmd(cmd, timeout, retries).strip()

  def ForwardRemove(self,
                    local,
                    timeout=DEFAULT_TIMEOUT,
                    retries=DEFAULT_RETRIES):
    """Remove a forward socket connection.

    Args:
      local: The host socket.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    """
    output = self._RunDeviceAdbCmd(
        ['forward', '--remove', str(local)], timeout, retries)
    if output:
      logger.warning('Unexpected output from "adb forward --remove": %s',
                     output)

  def Reverse(self,
              remote,
              local,
              allow_rebind=False,
              timeout=DEFAULT_TIMEOUT,
              retries=DEFAULT_RETRIES):
    """Forward socket connections from the remote socket to the local socket.

    Sockets are specified by one of:
      tcp:<port>
      localabstract:<unix domain socket name>
      localreserved:<unix domain socket name>
      localfilesystem:<unix domain socket name>

    Args:
      remote: The device socket.
      local: The host socket.
      allow_rebind: A boolean indicating whether adb may rebind a remote socket;
        otherwise, the default, an exception is raised if the remote socket is
        already being forwarded.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    Returns:
      The output of the "adb reverse" command as a string.
    Raises:
      DeviceVersionError if the device SDK version does not support reverse
        forwarding
    """
    self._CheckSdkVersion(version_codes.LOLLIPOP,
                          error_message='Reverse forwarding is not supported')
    cmd = ['reverse']
    if not allow_rebind:
      cmd.append('--no-rebind')
    cmd.extend([str(remote), str(local)])
    return self._RunDeviceAdbCmd(cmd, timeout, retries).strip()

  def ReverseRemove(self,
                    remote,
                    timeout=DEFAULT_TIMEOUT,
                    retries=DEFAULT_RETRIES):
    """Remove a reverse forward socket connection.

    Args:
      local: The device socket.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    Raises:
      DeviceVersionError if the device SDK version does not support reverse
        forwarding
    """
    self._CheckSdkVersion(version_codes.LOLLIPOP,
                          error_message='Reverse forwarding is not supported')
    output = self._RunDeviceAdbCmd(
        ['reverse', '--remove', str(remote)], timeout, retries)
    if output:
      logger.warning('Unexpected output from "adb reverse --remove": %s',
                     output)

  def ForwardList(self, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """List all currently forwarded socket connections.

    Args:
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    Returns:
      The output of adb forward --list as a string.
    """
    if (du_version.LooseVersion(self.Version()) >=
        du_version.LooseVersion('1.0.36')):
      # Starting in 1.0.36, this can occasionally fail with a protocol fault.
      # As this interrupts all connections with all devices, we instead just
      # return an empty list. This may give clients an inaccurate result, but
      # that's usually better than crashing the adb server.

      # TODO(jbudorick): Determine an appropriate upper version bound for this
      # once b/31811775 is fixed.
      return ''

    return self._RunDeviceAdbCmd(['forward', '--list'], timeout, retries)

  def JDWP(self, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """List of PIDs of processes hosting a JDWP transport.

    Args:
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.

    Returns:
      A list of PIDs as strings.
    """
    return [
        a.strip()
        for a in self._RunDeviceAdbCmd(['jdwp'], timeout, retries).split('\n')
    ]

  def Install(self,
              apk_path,
              forward_lock=False,
              allow_downgrade=False,
              reinstall=False,
              sd_card=False,
              streaming=None,
              timeout=DEFAULT_LONG_TIMEOUT,
              retries=DEFAULT_RETRIES,
              instant_app=False,
              force_queryable=False):
    """Install an apk on the device.

    Args:
      apk_path: Host path to the APK file.
      forward_lock: (optional) If set forward-locks the app.
      allow_downgrade: (optional) If set, allows for downgrades.
      reinstall: (optional) If set reinstalls the app, keeping its data.
      sd_card: (optional) If set installs on the SD card.
      streaming: (optional) If not set, use default way to install.
        If True, performs streaming install.
        If False, app is pushed to device and be installed from there.
        Note this option is not supported prior to adb version 1.0.40
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
      instant_app (optional): Install the APK as an instant app
      force_queryable (optional): Allows the installed application to
        be queryable by all other applications regardless of if they
        have declared the package as queryable in their manifests
    """
    VerifyLocalFileExists(apk_path)
    cmd = ['install']
    if forward_lock:
      cmd.append('-l')
    if reinstall:
      cmd.append('-r')
    if sd_card:
      cmd.append('-s')
    if allow_downgrade:
      cmd.append('-d')
    if instant_app:
      self._CheckSdkVersion(29, error_message='Instant apps not supported')
      cmd.append('--instant')
    if force_queryable:
      self._CheckSdkVersion(30, error_message='Force queryable not supported')
      cmd.append('--force-queryable')
    if streaming in (True, False):
      if (du_version.LooseVersion(self.Version()) <
          du_version.LooseVersion('1.0.40')):
        logging.warning(
            'adb: streaming options not supported prior to version 1.0.40 '
            '(current: %s)', self.Version())
      elif streaming:
        cmd.append('--streaming')
      else:
        cmd.append('--no-streaming')
    cmd.append(apk_path)
    output = self._RunDeviceAdbCmd(cmd, timeout, retries)
    if 'Success' not in output:
      raise device_errors.AdbCommandFailedError(
          cmd, output, device_serial=self._device_serial)

  def InstallMultiple(self,
                      apk_paths,
                      forward_lock=False,
                      reinstall=False,
                      sd_card=False,
                      allow_downgrade=False,
                      partial=False,
                      streaming=None,
                      timeout=DEFAULT_LONG_TIMEOUT,
                      retries=DEFAULT_RETRIES,
                      instant_app=False,
                      force_queryable=False):
    """Install an apk with splits on the device.

    Args:
      apk_paths: Host path to the APK file.
      forward_lock: (optional) If set forward-locks the app.
      reinstall: (optional) If set reinstalls the app, keeping its data.
      sd_card: (optional) If set installs on the SD card.
      allow_downgrade: (optional) Allow versionCode downgrade.
      partial: (optional) Package ID if apk_paths doesn't include all .apks.
      streaming: (optional) If not set, use default way to install.
        If True, performs streaming install.
        If False, app is pushed to device and be installed from there.
        Note this option is not supported prior to adb version 1.0.40
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
      instant_app (optional): Install the APK as an instant app
      force_queryable (optional): Allows the installed application to
        be queryable by all other applications regardless of if they
        have declared the package as queryable in their manifests
    """
    for path in apk_paths:
      VerifyLocalFileExists(path)
    cmd = ['install-multiple']
    if (du_version.LooseVersion(self.ReleaseVersion()) <
        du_version.LooseVersion('33.0.1')):
      # Workaround for http://issuetracker.google.com/218716282. In adb versions
      # before the one in platform-tools 33.0.1, the first arg is ignored for
      # install-multiple. Pass an extra arg in this case to avoid one of the
      # other arguments being ignored.
      cmd.append('unused-arg-workaround')
    if forward_lock:
      cmd.append('-l')
    if reinstall:
      cmd.append('-r')
    if sd_card:
      cmd.append('-s')
    if allow_downgrade:
      cmd.append('-d')
    if instant_app:
      self._CheckSdkVersion(29, error_message='Instant apps not supported')
      cmd.append('--instant')
    if force_queryable:
      self._CheckSdkVersion(30, error_message='Force queryable not supported')
      cmd.append('--force-queryable')
    if streaming in (True, False):
      if (du_version.LooseVersion(self.Version()) <
          du_version.LooseVersion('1.0.40')):
        logging.warning(
            'adb: streaming options not supported prior to version 1.0.40 '
            '(current: %s)', self.Version())
      elif streaming:
        cmd.append('--streaming')
      else:
        cmd.append('--no-streaming')
    if partial:
      cmd.extend(('-p', partial))
    cmd.extend(apk_paths)
    output = self._RunDeviceAdbCmd(cmd, timeout, retries)
    if 'Success' not in output:
      raise device_errors.AdbCommandFailedError(
          cmd, output, device_serial=self._device_serial)

  def Uninstall(self,
                package,
                keep_data=False,
                timeout=DEFAULT_TIMEOUT,
                retries=DEFAULT_RETRIES):
    """Remove the app |package| from the device.

    Args:
      package: The package to uninstall.
      keep_data: (optional) If set keep the data and cache directories.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    """
    cmd = ['uninstall']
    if keep_data:
      cmd.append('-k')
    cmd.append(package)
    output = self._RunDeviceAdbCmd(cmd, timeout, retries)
    if 'Failure' in output or 'Exception' in output:
      raise device_errors.AdbCommandFailedError(
          cmd, output, device_serial=self._device_serial)

  def Backup(self,
             path,
             packages=None,
             apk=False,
             shared=False,
             nosystem=True,
             include_all=False,
             timeout=DEFAULT_TIMEOUT,
             retries=DEFAULT_RETRIES):
    """Write an archive of the device's data to |path|.

    Args:
      path: Local path to store the backup file.
      packages: List of to packages to be backed up.
      apk: (optional) If set include the .apk files in the archive.
      shared: (optional) If set buckup the device's SD card.
      nosystem: (optional) If set exclude system applications.
      include_all: (optional) If set back up all installed applications and
        |packages| is optional.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    """
    cmd = ['backup', '-f', path]
    if apk:
      cmd.append('-apk')
    if shared:
      cmd.append('-shared')
    if nosystem:
      cmd.append('-nosystem')
    if include_all:
      cmd.append('-all')
    if packages:
      cmd.extend(packages)
    assert bool(packages) ^ bool(include_all), (
        'Provide \'packages\' or set \'include_all\' but not both.')
    ret = self._RunDeviceAdbCmd(cmd, timeout, retries)
    VerifyLocalFileExists(path)
    return ret

  def Restore(self, path, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """Restore device contents from the backup archive.

    Args:
      path: Host path to the backup archive.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    """
    VerifyLocalFileExists(path)
    self._RunDeviceAdbCmd(['restore'] + [path], timeout, retries)

  def WaitForDevice(self, timeout=DEFAULT_LONG_TIMEOUT,
                    retries=DEFAULT_RETRIES):
    """Block until the device is online.

    Args:
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    """
    self._RunDeviceAdbCmd(['wait-for-device'], timeout, retries)

  def GetState(self, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """Get device state.

    Args:
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.

    Returns:
      One of 'offline', 'bootloader', or 'device'.
    """
    # TODO(jbudorick): Revert to using get-state once it doesn't cause a
    # a protocol fault.
    # return self._RunDeviceAdbCmd(['get-state'], timeout, retries).strip()

    lines = self._RawDevices(timeout=timeout, retries=retries)
    for line in lines:
      if len(line) >= 2 and line[0] == self._device_serial:
        return line[1]
    return 'offline'

  def GetDevPath(self, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """Gets the device path.

    Args:
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.

    Returns:
      The device path (e.g. usb:3-4)
    """
    return self._RunDeviceAdbCmd(['get-devpath'], timeout, retries)

  def Remount(self, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """Remounts the /system partition on the device read-write."""
    self._RunDeviceAdbCmd(['remount'], timeout, retries)

  def Reboot(self,
             to_bootloader=False,
             timeout=DEFAULT_LONG_TIMEOUT,
             retries=DEFAULT_RETRIES):
    """Reboots the device.

    Args:
      to_bootloader: (optional) If set reboots to the bootloader.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    """
    if to_bootloader:
      cmd = ['reboot-bootloader']
    else:
      cmd = ['reboot']
    self._RunDeviceAdbCmd(cmd, timeout, retries)
    self.KillAllPersistentAdbs()

  def Root(self, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """Restarts the adbd daemon with root permissions, if possible.

    Args:
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.
    """
    try:
      output = self._RunDeviceAdbCmd(['root'], timeout, retries)
      # Need to kill persistent adbs after root command so it can restart.
      self.KillAllPersistentAdbs()
    except device_errors.AdbCommandFailedError as e:
      # For some devices, root can occasionally fail with this error and kick
      # the device into adb 'offline' mode. Assuming this is transient, try
      # waiting for the device to come back up before re-raising the exception
      # and proceeding with any retries.
      if 'unable to connect for root: closed' in e.output:
        self.WaitForDevice()
      raise
    if 'cannot' in output:
      raise device_errors.AdbCommandFailedError(
          ['root'], output, device_serial=self._device_serial)

  def Emu(self, cmd, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """Runs an emulator console command.

    See http://developer.android.com/tools/devices/emulator.html#console

    Args:
      cmd: The command to run on the emulator console.
      timeout: (optional) Timeout per try in seconds.
      retries: (optional) Number of retries to attempt.

    Returns:
      The output of the emulator console command.
    """
    if isinstance(cmd, six.string_types):
      cmd = [cmd]
    return self._RunDeviceAdbCmd(['emu'] + cmd, timeout, retries)

  def DisableVerity(self, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """Disable Marshmallow's Verity security feature.

    Returns:
      The output of the disable-verity command as a string.
    """
    output = self._RunDeviceAdbCmd(['disable-verity'], timeout, retries)
    if output and not _VERITY_DISABLE_RE.search(output):
      raise device_errors.AdbCommandFailedError(
          ['disable-verity'], output, device_serial=self._device_serial)
    return output

  def EnableVerity(self, timeout=DEFAULT_TIMEOUT, retries=DEFAULT_RETRIES):
    """Enable Marshmallow's Verity security feature.

    Returns:
      The output of the enable-verity command as a string.
    """
    output = self._RunDeviceAdbCmd(['enable-verity'], timeout, retries)
    if output and not _VERITY_ENABLE_RE.search(output):
      raise device_errors.AdbCommandFailedError(
          ['enable-verity'], output, device_serial=self._device_serial)
    return output

  def _get_a_shell(self):
    try:
      return self._idle_persistent_shells_queue.get(block=False)
    except Empty:
      self._lock.acquire()
      shell = self.PersistentShell(self._device_serial, registry=self)
      shell.Start()
      self._all_persistent_shells.append(shell)
      self._lock.release()
      return shell

  # Deprecated use device_utils#is_emulator instead.
  @property
  def is_emulator(self):
    return _EMULATOR_RE.match(self._device_serial)

  @property
  def is_ready(self):
    try:
      return self.GetState() == _READY_STATE
    except device_errors.CommandFailedError:
      return False
