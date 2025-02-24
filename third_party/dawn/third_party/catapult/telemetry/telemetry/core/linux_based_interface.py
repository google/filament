# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A wrapper around ssh for common operations on a Linux-based device"""
from __future__ import absolute_import
import logging
import os
import re
import shutil
import stat
import posixpath
import subprocess
import tempfile
import time

from devil.utils import cmd_helper
from telemetry.util import cmd_util

# Some developers' workflow includes running the Chrome process from
# /usr/local/... instead of the default location. We have to check for both
# paths in order to support this workflow.
CHROME_PROCESS_REGEX = [
    re.compile(r'^/opt/google/chrome/chrome '),
    re.compile(r'^/usr/local/?.*/chrome/chrome ')
]

_IGNORE_FILETYPES_FOR_MINIDUMP_PULLS = [
    '.lock',
    '.dat',
]


def _IsIgnoredFileType(filename):
  """Returns whether a given file should be ignored when pulling minidumps.

  Args:
    filename: A string containing the filename of the file to check.

  Returns:
    True if the file should be ignored, otherwise False.
  """
  for extension in _IGNORE_FILETYPES_FOR_MINIDUMP_PULLS:
    if filename.endswith(extension):
      return True
  return False


def RunCmd(args, cwd=None, quiet=False):
  """Synchronously runs a command in a subprocess, and returns its returncode.

  Args:
    args: list of program arguments as strings.
    cwd: string, directory in which command should be executed.
    quiet: boolean, whether or not to log the command execution.

  Returns:
    Return code of the executed program.
  """
  return cmd_util.RunCmd(args, cwd, quiet)


def GetAllCmdOutput(args, cwd=None, quiet=False):
  """Executes a command synchronously in a subprocess, returning its output.

  Args:
    args: list of program arguments as strings.
    cwd: string, directory to execute command in.
    quiet: boolean, whether or not to log command execution.

  Returns:
    Tuple of stdout, stderr of executed command.
  """
  # GetAllCmdOutput returns bytes on Python 3. As the downstream codes are
  # expecting strings, we decode the inpout here.
  stdout, stderr = cmd_util.GetAllCmdOutput(args, cwd, quiet)
  return (stdout.decode('utf-8'), stderr.decode('utf-8'))


def StartCmd(args, cwd=None, quiet=False, stdout=None, stderr=None, env=None):
  """Starts a command in a subprocess, returning a handle to it.

  Args:
    args: list of program arguments, as strings.
    cwd: string, directory in which command should be executed.
    quiet: boolean, whether or not to log command execution.
    stdout: optional file handle to point stdout to.
    stderr: optional file handle to point stderr to.
    env: optional environment (as dict[str, str]) in which to execute command.

  Returns:
    subprocess.Popen object pointing to executing subprocess.
  """
  return cmd_util.StartCmd(args, cwd=cwd, quiet=quiet, stdout=stdout,
                           stderr=stderr, env=env)


def HasSSH():
  return cmd_util.HasSSH()


class LoginException(Exception):
  pass


class KeylessLoginRequiredException(LoginException):
  pass


class DNSFailureException(LoginException):
  pass


def _Unquote(s):
  """Removes any trailing/leading single/double quotes from a string.

  No-ops if the given object is not a string or otherwise does not have a
  .strip() method.

  Args:
    s: The string to remove quotes from.

  Returns:
    |s| with trailing/leading quotes removed.
  """
  if not hasattr(s, 'strip'):
    return s
  # Repeated to handle both "'foo'" and '"foo"'
  return s.strip("\"'")


class LinuxBasedInterface:

  _DEFAULT_SSH_CONNECTION_TIMEOUT = 30
  _DEFAULT_SSH_COMMAND = [
      'ssh', '-o ForwardX11=no', '-o ForwardX11Trusted=no', '-n'
  ]

  # Login error messages.
  _HOST_KEY_ERROR = 'Host key verification failed'
  _TIMEOUT_ERROR = 'Operation timed out'
  _PRIV_KEY_PROTECTIONS_ERROR = 'UNPROTECTED PRIVATE KEY FILE!'
  _SSH_AUTH_ERROR = 'Permission denied (publickey,keyboard-interactive)'
  _HOSTNAME_RESOLUTION_ERROR = 'Could not resolve hostname'

  _REMOTE_USER = 'root'

  MINIDUMP_DIR = '~/.config/google-chrome/Crash Reports'

  _SCREENSHOT_BINARY = 'gnome-screenshot'
  _SCREENSHOT_TMP_DIR = '/tmp/log/screenshots/%s'

  def __init__(self, hostname=None, ssh_port=None, ssh_identity=None):
    self._hostname = hostname
    self._ssh_port = ssh_port

    # List of ports generated from GetRemotePort() that may not be in use yet.
    self._reserved_ports = []

    self._device_host_clock_offset = None
    self._master_connection_open = False
    self._disable_strict_filenames = False

    # Cached properties
    self._arch_name = None
    self._board = None
    self._device_type_name = None
    self._is_running_on_vm = None

    if self.local:
      return

    self._ssh_identity = None
    self._ssh_args = [
        '-o StrictHostKeyChecking=no', '-o KbdInteractiveAuthentication=no',
        '-o PreferredAuthentications=publickey',
        '-o UserKnownHostsFile=/dev/null', '-o ControlMaster=no', '-F/dev/null'
    ]

    if ssh_identity:
      self._ssh_identity = os.path.abspath(os.path.expanduser(ssh_identity))
      os.chmod(self._ssh_identity, stat.S_IREAD)

    # Since only one test will be run on a remote host at a time,
    # the control socket filename can be telemetry@hostname.
    self._ssh_control_file = '/tmp/' + 'telemetry' + '@' + self._hostname
    if os.path.exists(self._ssh_control_file):
      os.remove(self._ssh_control_file)
    self.OpenConnection()

  def GetDeviceTypeName(self):
    raise NotImplementedError

  def GetBoard(self):
    raise NotImplementedError

  def RestartUI(self):
    raise NotImplementedError

  @property
  def display(self):
    raise NotImplementedError

  def __enter__(self):
    return self

  def __exit__(self, *args):
    self.CloseConnection()

  @property
  def local(self):
    return not self._hostname

  @property
  def hostname(self):
    return self._hostname

  @property
  def ssh_port(self):
    return self._ssh_port

  @property
  def path(self):
    return posixpath

  def OpenConnection(self):
    """Opens a master connection to the device."""
    if self._master_connection_open or self.local:
      return
    # Establish master SSH connection using ControlPersist.
    with open(os.devnull, 'w') as devnull:
      subprocess.call(
          self.FormSSHCommandLine(['-M', '-o ControlPersist=yes']),
          stdin=devnull,
          stdout=devnull,
          stderr=devnull)
    self._master_connection_open = True

  def FormSSHCommandLine(self,
                         args,
                         extra_ssh_args=None,
                         port_forward=False,
                         connect_timeout=None,
                         env=None):
    """Constructs a subprocess-suitable command line for `ssh'.

    Args:
      args: command arguments as a list of strings.
      extra_ssh_args: Optional additional arguments to append to the SSH
        command, as a list of strings.
      port_forward: boolean, whether or not skipping the master connection
        is required (skips if port_forward=True. Default is False).
      connect_timeout: Optional integer timeout (in secs) to wait for the
        connection to establish.
      env: Optional dict[str,str] describing environment variables for command
        execution (not SSH execution).
    """

    def _sanitize(raw_value):
      return str(raw_value).replace('"', '\\"').replace('\r', '').replace(
          '\n', '').replace('%', '\\%')

    env_vars = []
    if env:
      for k, v in env.items():
        k = _sanitize(k)
        v = _sanitize(v)
        env_vars.append(f'{str(k)}="{str(v)}"')
    if self.local:
      # We run the command through the shell locally for consistency with
      # how commands are run through SSH (crbug.com/239161). This work
      # around will be unnecessary once we implement a persistent SSH
      # connection to run remote commands (crbug.com/239607).
      return env_vars + ['sh', '-c', " ".join(args)]

    full_args = self._DEFAULT_SSH_COMMAND[:]
    if connect_timeout:
      full_args += ['-o ConnectTimeout=%d' % connect_timeout]
    else:
      full_args += [
          '-o ConnectTimeout=%d' % self._DEFAULT_SSH_CONNECTION_TIMEOUT
      ]
    # As remote port forwarding might conflict with the control socket
    # sharing, skip the control socket args if it is for remote port forwarding.
    if not port_forward:
      full_args += ['-S', self._ssh_control_file]
    full_args += self._ssh_args
    if self._ssh_identity is not None:
      full_args.extend(['-i', self._ssh_identity])
    if extra_ssh_args:
      full_args.extend(extra_ssh_args)
    full_args.append('%s@%s' % (self._REMOTE_USER, self._hostname))
    full_args.append('-p%d' % self._ssh_port)
    full_args.extend(env_vars)
    full_args.extend(args)
    return full_args

  def _FormSCPCommandLine(self, src, dst, extra_scp_args=None):
    """Constructs a subprocess-suitable command line for `scp'.

    Note: this function is not designed to work with IPv6 addresses, which need
    to have their addresses enclosed in brackets and a '-6' flag supplied
    in order to be properly parsed by `scp'.
    """
    assert not self.local, "Cannot use SCP on local target."

    # -C enables compression.
    args = ['scp', '-C', '-P', str(self._ssh_port)] + self._ssh_args
    if self._ssh_identity:
      args.extend(['-i', self._ssh_identity])
    if extra_scp_args:
      args.extend(extra_scp_args)
    args += [src, dst]
    return args

  def _FormSCPToRemote(self,
                       source,
                       remote_dest,
                       extra_scp_args=None,
                       user='root'):
    return self._FormSCPCommandLine(
        source,
        '%s@%s:%s' % (user, self._hostname, remote_dest),
        extra_scp_args=extra_scp_args)

  def _FormSCPFromRemote(self,
                         remote_source,
                         dest,
                         extra_scp_args=None,
                         user='root'):
    return self._FormSCPCommandLine(
        '%s@%s:%s' % (user, self._hostname, remote_source),
        dest,
        extra_scp_args=extra_scp_args)

  def _RemoveSSHWarnings(self, to_clean):
    """Removes specific ssh warning lines from a string.

    Args:
      to_clean: A string that may be containing multiple lines.

    Returns:
      A copy of to_clean with all the Warning lines removed.
    """
    if isinstance(to_clean, bytes):
      to_clean = to_clean.decode('utf-8')
    # Remove the Warning about connecting to a new host for the first time.
    return re.sub(
        r'Warning: Permanently added [^\n]* to the list of known hosts.\s\n',
        '', to_clean)

  def RunCmdOnDevice(self,
                     args,
                     cwd=None,
                     quiet=False,
                     connect_timeout=None,
                     port_forward=False,
                     env=None):
    stdout, stderr = GetAllCmdOutput(
        self.FormSSHCommandLine(
            args,
            connect_timeout=connect_timeout,
            port_forward=port_forward,
            env=env),
        cwd=cwd,
        quiet=quiet)
    # The initial login will add the host to the hosts file but will also print
    # a warning to stderr that we need to remove.
    stderr = self._RemoveSSHWarnings(stderr)
    return stdout, stderr

  def RunCmdOnDeviceWithRC(self,
                           args,
                           cwd=None,
                           quiet=False,
                           connect_timeout=None):
    popen = self.StartCmdOnDevice(
        args, cwd=cwd, quiet=quiet, connect_timeout=connect_timeout)
    popen.wait()
    stdout, stderr = popen.communicate()
    if isinstance(stdout, bytes):
      stdout = stdout.decode('utf-8')
      stderr = stderr.decode('utf-8')
    stderr = self._RemoveSSHWarnings(stderr)
    return popen.returncode, stdout, stderr

  def StartCmdOnDevice(self,
                       args,
                       cwd=None,
                       quiet=False,
                       connect_timeout=None,
                       stdout=None,
                       stderr=None,
                       env=None):
    return StartCmd(
        self.FormSSHCommandLine(args, connect_timeout=connect_timeout, env=env),
        cwd=cwd,
        quiet=quiet,
        stdout=stdout,
        stderr=stderr)

  def TryLogin(self):
    logging.debug('TryLogin()')
    assert not self.local
    # Initial connection may take a bit to establish (especially if the
    # VM/device just booted up). So bump the default timeout.
    stdout, stderr = self.RunCmdOnDevice(['echo', '$USER'],
                                         quiet=True,
                                         connect_timeout=60)
    stdout, stderr = stdout.strip(), stderr.strip()
    if stderr != '':
      if self._HOST_KEY_ERROR in stderr:
        raise LoginException(
            ('%s host key verification failed. ' +
             'SSH to it manually to fix connectivity.') % self._hostname)
      if self._TIMEOUT_ERROR in stderr:
        raise LoginException('Timed out while logging into %s' % self._hostname)
      if self._PRIV_KEY_PROTECTIONS_ERROR in stderr:
        raise LoginException('Permissions for %s are too open. To fix this,\n'
                             'chmod 600 %s' %
                             (self._ssh_identity, self._ssh_identity))
      if self._SSH_AUTH_ERROR in stderr:
        raise KeylessLoginRequiredException('Need to set up ssh auth for %s' %
                                            self._hostname)
      if self._HOSTNAME_RESOLUTION_ERROR in stderr:
        raise DNSFailureException('Unable to resolve the hostname for: %s' %
                                  self._hostname)
      raise LoginException('While logging into %s, got %s' %
                           (self._hostname, stderr))
    if stdout != self._REMOTE_USER:
      raise LoginException('Logged into %s, expected $USER=%s, but got %s.' %
                           (self._hostname, self._REMOTE_USER, stdout))

  def FileExistsOnDevice(self, file_name):
    return self.TestFile(file_name, exists=True)

  def IsFile(self, file_name):
    logging.debug('Verify: %s', file_name)
    return self.TestFile(file_name, is_file=True)

  def IsDir(self, file_name):
    return self.TestFile(file_name, is_dir=True)

  def TestFile(self, path, exists=False, is_file=False, is_dir=False):
    assert exists or is_file or is_dir, 'Must specify an existence condition'
    assert not (is_file and is_dir), 'Cannot specify both is_file and is_dir'
    if exists:
      condition = '-e'

    if is_file:
      condition = '-f'

    if is_dir:
      condition = '-d'

    stdout, stderr = self.RunCmdOnDevice(
        ['if', 'test', condition, path, ';', 'then', 'echo', '1', ';', 'fi'])
    if stderr != '':
      if "Connection timed out" in stderr:
        raise OSError('Machine wasn\'t responding to ssh: %s' % stderr)
      raise OSError('Unexpected error: %s' % stderr)
    test_result = stdout == '1\n'
    logging.debug("TestFile(<text>, %s, %s)->%s" %
                  (path, condition, test_result))
    return test_result

  def MkdTemp(self, prefix=None):
    cmd = ['mktemp', '-d']
    if prefix:
      cmd.append(prefix)
    for _ in range(5):
      rc, stdout, stderr = self.RunCmdOnDeviceWithRC(cmd)
      if rc == 0:
        return stdout.strip()
    raise OSError(
        'Could not create a temporary directory, %s, %s' % (stdout.strip(),
                                                            stderr.strip()))

  def PushFile(self, filename, remote_filename):
    assert os.path.exists(filename), 'Cannot push a file that does not exist'
    if self.local:
      args = ['cp', '-r', filename, remote_filename]
      _, stderr = GetAllCmdOutput(args, quiet=True)
      if stderr != '':
        raise OSError('No such file or directory %s' % stderr)
      return

    args = self._FormSCPToRemote(
        os.path.abspath(filename),
        remote_filename,
        extra_scp_args=['-r'],
        user=self._REMOTE_USER)

    _, stderr = GetAllCmdOutput(args, quiet=False)
    stderr = self._RemoveSSHWarnings(stderr)
    if stderr != '':
      raise OSError('No such file or directory %s' % stderr)

  def PushContents(self, text, remote_filename):
    logging.debug("PushContents(<text>, %s)" % remote_filename)
    with tempfile.NamedTemporaryFile(mode='w+') as f:
      f.write(text)
      f.flush()
      self.PushFile(f.name, remote_filename)

  def GetFile(self, filename, destfile=None):
    """Copies a remote file |filename| on the device to a local file |destfile|.

    Args:
      filename: The name of the remote source file.
      destfile: The name of the file to copy to, and if it is not specified
        then it is the basename of the source file.

    """
    # TODO this is new behavior based on the description.
    if destfile is None:
      destfile = os.path.basename(filename)
    destfile = os.path.abspath(destfile)
    logging.debug("GetFile(%s, %s)" % (filename, destfile))

    if self.local:
      filename = _Unquote(filename)
      destfile = _Unquote(destfile)
      if destfile is not None and destfile != filename:
        shutil.copyfile(filename, destfile)
        return
      # TODO: This is only raised if the files match.
      raise OSError('No such file or directory %s' % filename)

    extra_args = ['-T'] if self._disable_strict_filenames else []
    args = self._FormSCPFromRemote(
        filename, destfile, extra_scp_args=extra_args, user=self._REMOTE_USER)

    _, stderr = GetAllCmdOutput(args, quiet=True)
    stderr = self._RemoveSSHWarnings(stderr)
    # This is a workaround for a bug in SCP that was added ~January 2019, where
    # strict filename checking can erroneously reject valid filenames. Passing
    # -T goes back to the older behavior, but scp doesn't have a good way of
    # checking the version, so we can't pass -T the first time based on that.
    # Instead, try without -T and retry with -T if the error message is
    # appropriate. See
    # https://unix.stackexchange.com/questions/499958/why-does-scps-strict-filename-checking-reject-quoted-last-component-but-not-oth
    # for more information.
    if ('filename does not match request' in stderr
        and not self._disable_strict_filenames):
      self._disable_strict_filenames = True
      args = self._FormSCPFromRemote(
          filename, destfile, extra_scp_args=['-T'], user=self._REMOTE_USER)
      _, stderr = GetAllCmdOutput(args, quiet=True)
      stderr = self._RemoveSSHWarnings(stderr)
    if stderr != '':
      raise OSError('No such file or directory %s' % stderr)

  def GetFileContents(self, filename):
    """Get the contents of a file on the device.

    Args:
      filename: The name of the file on the device.

    Returns:
      A string containing the contents of the file.
    """
    logging.debug("GetFileContents(%s)" % (filename))
    with tempfile.NamedTemporaryFile(mode='w') as t:
      self.GetFile(filename, t.name)
      with open(t.name, 'r', encoding='UTF-8') as f2:
        res = f2.read()
        logging.debug("GetFileContents(%s)->%s" % (filename, res))
        return res

  def GetDeviceHostClockOffset(self):
    """Returns the difference between the device and host clocks."""
    if self._device_host_clock_offset is None:
      device_time, _ = self.RunCmdOnDevice(['date', '+%s'])
      host_time = time.time()
      self._device_host_clock_offset = int(int(device_time.strip()) - host_time)
    return self._device_host_clock_offset

  def HasSystemd(self):
    """Return True or False to indicate if systemd is used.

    Note: This function checks to see if the 'systemctl' utility
    is installed. This is only installed along with the systemd daemon.
    """
    _, stderr = self.RunCmdOnDevice(['systemctl'], quiet=True)
    return stderr == ''

  def ListProcesses(self):
    """Returns (pid, cmd, ppid, state) of all processes on the device."""
    stdout, stderr = self.RunCmdOnDevice(
        ['/bin/ps', '--no-headers', '-A', '-o', 'pid,ppid,args:4096,state'],
        quiet=True)
    assert stderr == '', stderr
    procs = []
    for l in stdout.split('\n'):
      if l == '':
        continue
      m = re.match(r'^\s*(\d+)\s+(\d+)\s+(.+)\s+(.+)', l, re.DOTALL)
      assert m
      procs.append(
          (int(m.group(1)), m.group(3).rstrip(), int(m.group(2)), m.group(4)))
    logging.debug("ListProcesses(<predicate>)->[%i processes]" % len(procs))
    return procs

  def RmRF(self, filename):
    logging.debug("rm -rf %s" % filename)
    self.RunCmdOnDevice(['rm', '-rf', filename], quiet=True)

  def Chown(self, filename, owner, group):
    self.RunCmdOnDevice(['chown', '-R', f'{owner}:{group}', filename])

  def KillAllMatching(self, predicate):
    kills = ['kill', '-KILL']
    for pid, cmd, _, _ in self.ListProcesses():
      if predicate(cmd):
        logging.info('Killing %s, pid %d', cmd, pid)
        kills.append(str(pid))
    logging.debug("KillAllMatching(<predicate>)->%i" % (len(kills) - 2))
    if len(kills) > 2:
      self.RunCmdOnDevice(kills, quiet=True)
    return len(kills) - 2

  def IsServiceRunning(self, service_name):
    """Check with the init daemon if the given service is running."""
    stdout, stderr = 'stdout', 'system does not have systemd'
    if self.HasSystemd():
      # Querying for the pid of the service will return 'MainPID=0' if
      # the service is not running.
      stdout, stderr = self.RunCmdOnDevice(
          ['systemctl', 'show', '-p', 'MainPID', service_name], quiet=True)
      running = int(stdout.split('=')[1]) != 0
    assert stderr == '', stderr
    logging.debug("IsServiceRunning(%s)->%s" % (service_name, running))
    return running

  def GetRemotePort(self):
    netstat = self.RunCmdOnDevice(['netstat', '-ant'])
    netstat = netstat[0].split('\n')
    ports_in_use = []

    for line in netstat[2:]:
      if not line:
        continue
      address_in_use = line.split()[3]
      port_in_use = address_in_use.split(':')[-1]
      ports_in_use.append(int(port_in_use))

    ports_in_use.extend(self._reserved_ports)

    new_port = sorted(ports_in_use)[-1] + 1
    self._reserved_ports.append(new_port)

    return new_port

  def IsHTTPServerRunningOnPort(self, port):
    # Only tests ipv4 address.
    wget_output = self.RunCmdOnDevice(
        ['wget', 'localhost:%i' % (port), '-T1', '-t1', '-4'])

    if 'Connection refused' in wget_output[1]:
      return False

    return True

  def TakeScreenshotWithPrefix(self, screenshot_prefix):
    """Takes a screenshot, useful for debugging failures."""
    screenshot_dir = '/tmp/telemetry/screenshots/'
    screenshot_ext = '.png'

    self.RunCmdOnDevice(['mkdir', '-p', screenshot_dir])
    # Large number of screenshots can increase hardware lab bandwidth
    # dramatically, so keep this number low. crbug.com/524814.
    for i in range(2):
      screenshot_file = ('%s%s-%d%s' %
                         (screenshot_dir, screenshot_prefix, i, screenshot_ext))
      if not self.FileExistsOnDevice(screenshot_file):
        return self.TakeScreenshot(screenshot_file)
    logging.warning('screenshot directory full.')
    return False

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

  def ExpandUser(self, path):
    if self.local:
      return os.path.expanduser(path)

    # There is the the case where ~user is used in a path, but this will not
    # be supported for now.
    if '~' in path and '~/' not in path:
      raise ValueError(f'Cannot expand path {path}')

    stdout, _ = self.RunCmdOnDevice(['echo', '$HOME'], quiet=True)
    home = stdout.strip()
    path = path.replace('~/', '')
    return os.path.join(home, path)

  def PullDumps(self, host_dir):
    """Pulls any minidumps from the device/emulator to the host.

    Skips pulling any dumps that have already been pulled. The modification time
    of any pulled dumps will be set to the modification time of the dump on the
    device/emulator, offset by any difference in clocks between the device and
    host.

    Args:
      host_dir: The directory on the host where the dumps will be copied to.
    """
    # The device/emulator's clock might be off from the host, so calculate an
    # offset that can be added to the host time to get the corresponding device
    # time.
    # The offset is (device_time - host_time), so a positive value means that
    # the device clock is ahead.
    time_offset = self.GetDeviceHostClockOffset()

    stdout, _ = self.RunCmdOnDevice([
        'ls', '-1',
        cmd_helper.SingleQuote(self.ExpandUser(self.MINIDUMP_DIR))
    ])
    device_dumps = stdout.splitlines()
    for dump_filename in device_dumps:
      host_path = os.path.join(host_dir, dump_filename)
      # Skip any ignored files since they're not useful and could be deleted by
      # the time we try to pull them.
      if _IsIgnoredFileType(dump_filename):
        continue
      if os.path.exists(host_path):
        continue
      device_path = cmd_helper.SingleQuote(
          posixpath.join(self.MINIDUMP_DIR, dump_filename))

      # Skip any directories that happen to be in the list.
      if self.IsDir(device_path):
        continue

      # Skip any files that have a corresponding .lock file, as that implies the
      # file hasn't been fully written to disk yet.
      device_lock_path = cmd_helper.SingleQuote(
          posixpath.join(self.MINIDUMP_DIR, dump_filename + '.lock'))
      if self.FileExistsOnDevice(device_lock_path):
        logging.debug('Not pulling file %s because a .lock file exists for it',
                      device_path)
        continue
      try:
        self.GetFile(device_path, host_path)
      except Exception as e:  # pylint: disable=broad-except
        logging.error('Failed to get file %s: %s', device_path, e)
        continue
      # Set the local version's modification time to the device's.
      stdout, _ = self.RunCmdOnDevice(
          ['ls', '--time-style', '+%s', '-l', device_path])
      stdout = stdout.strip()
      # We expect whitespace-separated fields in this order:
      # mode, links, owner, group, size, mtime, filename.
      # Offset by the difference of the device and host clocks.
      device_mtime = int(stdout.split()[5])
      host_mtime = device_mtime - time_offset
      os.utime(host_path, (host_mtime, host_mtime))

  def _ScreenshotCmd(self, file_path):
    """Constructs screenshot cmd for device to save screenshot at |file_path|.

    Args:
      file_path: string, file path on DUT to save screenshot.
    Returns:
      list of arguments for the screenshot cmd.
    """
    return [self._SCREENSHOT_BINARY, '-f', file_path]

  def CanTakeScreenshot(self):
    cmd = ['which', self._SCREENSHOT_BINARY]
    rc, _, _  = self.RunCmdOnDeviceWithRC(cmd)
    return rc == 0

  def TakeScreenshot(self, file_path):
    """Takes a screenshot, saves to |file_path|.

    Also Saves a copy of the screenshot to //var/log/screenshots for additional
    debug scenarios.

    If running in remote mode, also pulls the file to the same location on the
    host.

    Returns:
      True if the screenshot was taken successfully, otherwise False.
    """
    if not self.CanTakeScreenshot():
      logging.warning('Cannot take screenshot on this device.')
      return False

    # When running remotely, taking a screenshot to the specified |file_path|
    # may fail due to differences between the device and host. We also want
    # to save a copy to /var/log/ on the device, as it is saved by CrOS bots.
    # Address both by taking the screenshot to /var/log/ and either copying
    # to the correct location in local mode or pulling to the correct location
    # in remote mode.
    basename = os.path.basename(file_path)
    var_path = self._SCREENSHOT_TMP_DIR % basename
    dir_name = os.path.dirname(file_path)
    self.RunCmdOnDevice(['mkdir', '-p', self.path.dirname(var_path)])
    screenshot_cmd = self._ScreenshotCmd(var_path)
    screenshot_cmd.extend(['&&', 'echo',
        'screenshot return value:$?'
    ])
    stdout, stderr = self.RunCmdOnDevice(screenshot_cmd)

    if self.local:
      self.RunCmdOnDevice(['mkdir', '-p', dir_name])
      self.RunCmdOnDevice(['cp', var_path, file_path])
    else:
      try:
        if not os.path.exists(dir_name):
          os.makedirs(dir_name)
        self.GetFile(var_path, file_path)
      except OSError as e:
        logging.error('Unable to pull screenshot file %s to %s: %s', var_path,
                      file_path, e)
        logging.error('Screenshot capture output: %s\n%s', stdout, stderr)
    return 'screenshot return value:0' in stdout

  def CloseConnection(self):
    if not self.local and self._master_connection_open:
      with open(os.devnull, 'w') as devnull:
        subprocess.call(
            self.FormSSHCommandLine(['-O', 'exit', self._hostname]),
            stdout=devnull,
            stderr=devnull)
      self._master_connection_open = False
