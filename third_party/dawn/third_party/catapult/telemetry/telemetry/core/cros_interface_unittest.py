# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# TODO(nduca): Rewrite what some of these tests to use mocks instead of
# actually talking to the device. This would improve our coverage quite
# a bit.

from __future__ import absolute_import
import os
import shutil
import socket
import tempfile
import unittest
from unittest import mock

from devil.utils import cmd_helper
from telemetry.core import cros_interface
from telemetry import decorators
from telemetry.internal.forwarders import linux_based_forwarder
from telemetry.testing import options_for_unittests


class CrOSInterfaceTest(unittest.TestCase):

  def _GetCRI(self):
    remote = options_for_unittests.GetCopy().remote
    remote_ssh_port = options_for_unittests.GetCopy().remote_ssh_port
    return cros_interface.CrOSInterface(
        remote, remote_ssh_port,
        options_for_unittests.GetCopy().ssh_identity)

  @decorators.Enabled('chromeos')
  def testPushContents(self):
    with self._GetCRI() as cri:
      tmp_file = '/tmp/testPushContents'
      test_contents = 'hello world'
      cri.RmRF(tmp_file)
      cri.PushContents(test_contents, tmp_file)
      contents = cri.GetFileContents(tmp_file)
      self.assertEqual(contents, test_contents)

  @decorators.Enabled('chromeos')
  def testExists(self):
    with self._GetCRI() as cri:
      self.assertTrue(cri.FileExistsOnDevice('/proc/cpuinfo'))
      self.assertTrue(cri.FileExistsOnDevice('/etc/passwd'))
      self.assertFalse(cri.FileExistsOnDevice('/etc/sdlfsdjflskfjsflj'))

  @decorators.Enabled('chromeos')
  def testGetFileContents(self):
    with self._GetCRI() as cri:
      hosts = cri.GetFileContents('/etc/lsb-release')
      self.assertTrue('CHROMEOS' in hosts)

  @decorators.Enabled('chromeos')
  def testPullDumps(self):
    """Test that minidumps can be pulled off the device and their mtimes set."""
    tempdir = tempfile.mkdtemp()
    try:
      with self._GetCRI() as cri:
        time_offset = cri.GetDeviceHostClockOffset()
        # Make sure we don't end up getting a negative timestamp when we pull
        # the dump.
        ts = abs(time_offset) + 1
        remote_path = '/tmp/dumps/'
        cri.MINIDUMP_DIR = remote_path
        cri.RunCmdOnDevice(['mkdir', '-p', remote_path])
        # Set the mtime to one second after the epoch
        cri.RunCmdOnDevice(
            ['touch', '-d', '@%d' % ts, remote_path + 'test_dump'])
        try:
          cri.PullDumps(tempdir)
        finally:
          cri.RmRF(remote_path)
        local_path = os.path.join(tempdir, 'test_dump')
        self.assertTrue(os.path.exists(local_path))
        self.assertEqual(os.path.getmtime(local_path), ts - time_offset)
    finally:
      shutil.rmtree(tempdir)

  @decorators.Enabled('chromeos')
  def testPullDumpsOnlyNew(self):
    """Tests that a minidump is not pulled to the host if it already exists."""
    tempdir = tempfile.mkdtemp()
    with open(os.path.join(tempdir, 'old_dump'), 'w'):
      pass
    try:
      with self._GetCRI() as cri:
        time_offset = cri.GetDeviceHostClockOffset()
        # Make sure we don't end up getting a negative timestamp when we pull
        # the dump.
        ts = abs(time_offset) + 1
        remote_path = '/tmp/dumps/'
        cri.MINIDUMP_DIR = remote_path
        cri.RunCmdOnDevice(['mkdir', '-p', remote_path])
        # Set the mtime to one second after the epoch
        cri.RunCmdOnDevice(
            ['touch', '-d', '@%d' % ts, remote_path + 'old_dump'])
        cri.RunCmdOnDevice(
            ['touch', '-d', '@%d' % ts, remote_path + 'new_dump'])
        cri.PullDumps(tempdir)
        cri.RmRF(remote_path)
        # Only the file that didn't already exist locally should have been
        # pulled.
        local_path = os.path.join(tempdir, 'old_dump')
        self.assertTrue(os.path.exists(local_path))
        self.assertNotEqual(os.path.getmtime(local_path), ts - time_offset)
        local_path = os.path.join(tempdir, 'new_dump')
        self.assertTrue(os.path.exists(local_path))
        self.assertEqual(os.path.getmtime(local_path), ts - time_offset)
    finally:
      shutil.rmtree(tempdir)

  @decorators.Enabled('chromeos')
  def testPullDumpsDirectoriesIgnored(self):
    """Tests that directories are ignored when pulling minidumps."""
    tempdir = tempfile.mkdtemp()
    try:
      with self._GetCRI() as cri:
        remote_path = '/tmp/dumps/'
        cri.MINIDUMP_DIR = remote_path
        cri.RunCmdOnDevice(['mkdir', '-p', remote_path + 'test_dir'])
        # Set the mtime to one second after the epoch
        cri.RunCmdOnDevice(['touch', remote_path + 'test_dump'])
        try:
          cri.PullDumps(tempdir)
        finally:
          cri.RmRF(remote_path)
        # We should have pulled the dump, but not the directory.
        self.assertEqual(os.listdir(tempdir), ['test_dump'])
    finally:
      shutil.rmtree(tempdir)

  @decorators.Enabled('chromeos')
  def testPullDumpsIgnoresLockedFiles(self):
    """Tests that files with associated .lock files are not pulled."""
    tempdir = tempfile.mkdtemp()
    try:
      with self._GetCRI() as cri:
        remote_path = '/tmp/dumps/'
        cri.MINIDUMP_DIR = remote_path
        cri.RunCmdOnDevice(['mkdir', '-p', remote_path])
        cri.RunCmdOnDevice(
            ['touch', remote_path + 'test_dump'])
        cri.RunCmdOnDevice(
            ['touch', remote_path + 'locked_dump'])
        cri.RunCmdOnDevice(
            ['touch', remote_path + 'locked_dump.lock'])
        try:
          cri.PullDumps(tempdir)
        finally:
          cri.RmRF(remote_path)
        local_path = os.path.join(tempdir, 'test_dump')
        self.assertTrue(os.path.exists(local_path))
        local_path = os.path.join(tempdir, 'locked_dump')
        self.assertFalse(os.path.exists(local_path))
    finally:
      shutil.rmtree(tempdir)

  @decorators.Enabled('chromeos')
  def testPullDumpsIgnoredFiletype(self):
    """Tests that ignored filetypes are not pulled when pulling minidumps."""
    tempdir = tempfile.mkdtemp()
    try:
      with self._GetCRI() as cri:
        remote_path = '/tmp/dumps/'
        cri.MINIDUMP_DIR = remote_path
        cri.RunCmdOnDevice(['mkdir', '-p', remote_path])
        cri.RunCmdOnDevice(
            ['touch', remote_path + 'test_dump'])
        for ignored_type in cros_interface._IGNORE_FILETYPES_FOR_MINIDUMP_PULLS:
          cri.RunCmdOnDevice(
              ['touch', remote_path + 'test_file' + ignored_type])
        try:
          cri.PullDumps(tempdir)
        finally:
          cri.RmRF(remote_path)
        local_path = os.path.join(tempdir, 'test_dump')
        self.assertTrue(os.path.exists(local_path))
        for ignored_type in cros_interface._IGNORE_FILETYPES_FOR_MINIDUMP_PULLS:
          local_path = os.path.join(tempdir, 'test_file' + ignored_type)
          self.assertFalse(os.path.exists(local_path))
    finally:
      shutil.rmtree(tempdir)

  @decorators.Enabled('chromeos')
  @mock.patch.object(cros_interface.CrOSInterface, 'GetFile')
  def testPullDumpsHandlesFailedFilePulls(self, get_file_mock):
    """Tests that pulling minidumps does not explode if a file pull fails."""
    get_file_mock.side_effect = IOError('File go bye bye.')
    tempdir = tempfile.mkdtemp()
    try:
      with self._GetCRI() as cri:
        remote_path = '/tmp/dumps/'
        cri.MINIDUMP_DIR = remote_path
        cri.RunCmdOnDevice(['mkdir', '-p', remote_path])
        cri.RunCmdOnDevice(
            ['touch', remote_path + 'test_dump'])
        try:
          cri.PullDumps(tempdir)
        finally:
          cri.RmRF(remote_path)
        get_file_mock.assert_called_once_with(
            remote_path + 'test_dump', mock.ANY)
    finally:
      shutil.rmtree(tempdir)

  @decorators.Enabled('chromeos')
  def testGetFile(self):  # pylint: disable=no-self-use
    with self._GetCRI() as cri:
      f = tempfile.NamedTemporaryFile()
      cri.GetFile('/etc/lsb-release', f.name)
      with open(f.name, 'r') as f2:
        res = f2.read()
        self.assertTrue('CHROMEOS' in res)

  @decorators.Enabled('chromeos')
  def testGetFileQuotes(self):  # pylint: disable=no-self-use
    with self._GetCRI() as cri:
      f = tempfile.NamedTemporaryFile()
      cri.GetFile(cmd_helper.SingleQuote('/etc/lsb-release'), f.name)
      with open(f.name, 'r') as f2:
        res = f2.read()
        self.assertTrue('CHROMEOS' in res)

  @decorators.Enabled('chromeos')
  def testGetFileNonExistent(self):
    with self._GetCRI() as cri:
      f = '/tmp/testGetFile'  # A path that can be created on the device.
      cri.PushContents('testGetFileNonExistent', f)
      cri.RmRF(f)
      self.assertRaises(OSError, lambda: cri.GetFile(f))

  @decorators.Enabled('chromeos')
  def testIsServiceRunning(self):
    with self._GetCRI() as cri:
      self.assertTrue(cri.IsServiceRunning('openssh-server'))

  # TODO(crbug.com/799484): Fix this test.
  @decorators.Disabled('all')
  def testGetRemotePortAndIsHTTPServerRunningOnPort(self):
    with self._GetCRI() as cri:
      # Create local server.
      sock = socket.socket()
      sock.bind(('', 0))
      port = sock.getsockname()[1]
      sock.listen(0)

      # Get remote port and ensure that it was unused.
      remote_port = cri.GetRemotePort()
      self.assertFalse(cri.IsHTTPServerRunningOnPort(remote_port))

      # Forward local server's port to remote device's remote_port.
      forwarder = linux_based_forwarder.LinuxBasedForwarderFactory(cri).Create(
          local_port=port, remote_port=remote_port)

      # At this point, remote device should be able to connect to local server.
      self.assertTrue(cri.IsHTTPServerRunningOnPort(remote_port))

      # Next remote port shouldn't be the same as remote_port, since remote_port
      # is now in use.
      self.assertTrue(cri.GetRemotePort() != remote_port)

      # Close forwarder and local server ports.
      forwarder.Close()
      sock.close()

      # Device should no longer be able to connect to remote_port since it is no
      # longer in use.
      self.assertFalse(cri.IsHTTPServerRunningOnPort(remote_port))

  @decorators.Enabled('chromeos')
  def testGetRemotePortReservedPorts(self):
    with self._GetCRI() as cri:
      # Should return 2 separate ports even though the first one isn't
      # technically being used yet.
      remote_port_1 = cri.GetRemotePort()
      remote_port_2 = cri.GetRemotePort()

      self.assertTrue(remote_port_1 != remote_port_2)

  @decorators.Enabled('chromeos')
  def testTakeScreenshotWithPrefix(self):
    with self._GetCRI() as cri:
      def _Cleanup():
        cri.RmRF('/tmp/telemetry/screenshots/test-prefix*')
        cri.RmRF('/var/log/screenshots/test-prefix*')

      _Cleanup()
      self.assertTrue(cri.TakeScreenshotWithPrefix('test-prefix'))
      screenshot_file = '/tmp/telemetry/screenshots/test-prefix-0.png'
      self.assertTrue(
          cri.FileExistsOnDevice('/var/log/screenshots/test-prefix-0.png'))
      # Ensure we've pulled the screenshot to the host if we're running in
      # remote mode or copied to the correct location in local mode.
      self.assertTrue(os.path.exists(screenshot_file))
      # Ensure we actually have some amount of data.
      self.assertTrue(os.path.getsize(screenshot_file) > 0)
      _Cleanup()

  @decorators.Enabled('chromeos')
  def testLsbReleaseValue(self):
    with self._GetCRI() as cri:
      build_num = cri.LsbReleaseValue('CHROMEOS_RELEASE_BUILD_NUMBER', None)
      self.assertTrue(build_num.isdigit())
      device_type = cri.GetDeviceTypeName()
      self.assertTrue(device_type.isalpha())

  @decorators.Enabled('chromeos')
  def testGetBoard(self):
    # All devices, including VMs, should have board names.
    self.assertIsNotNone(self._GetCRI().GetBoard())

  @decorators.Enabled('chromeos')
  def testEscapeCmdArguments(self):
    """Commands and their arguments that are executed through the cros
    interface should follow bash syntax. This test needs to run on remotely
    and locally on the device to check for consistency.
    """
    options = options_for_unittests.GetCopy()
    with cros_interface.CrOSInterface(options.remote,
                                      options.remote_ssh_port,
                                      options.ssh_identity) as cri:

      # Check arguments with no special characters
      stdout, _ = cri.RunCmdOnDevice(['echo', '--arg1=value1', '--arg2=value2',
                                      '--arg3="value3"'])
      assert stdout.strip() == '--arg1=value1 --arg2=value2 --arg3=value3'

      # Check argument with special characters escaped
      stdout, _ = cri.RunCmdOnDevice(['echo', '--arg=A\\; echo \\"B\\"'])
      assert stdout.strip() == '--arg=A; echo "B"'

      # Check argument with special characters in quotes
      stdout, _ = cri.RunCmdOnDevice(['echo', "--arg='$HOME;;$PATH'"])
      assert stdout.strip() == "--arg=$HOME;;$PATH"

  @decorators.Enabled('chromeos')
  def testStartCmdOnDevice(self):
    options = options_for_unittests.GetCopy()
    with cros_interface.CrOSInterface(options.remote,
                                      options.remote_ssh_port,
                                      options.ssh_identity) as cri:
      p = cri.StartCmdOnDevice(['true'])
      p.wait()
      self.assertEqual(p.returncode, 0)

      p = cri.StartCmdOnDevice(['false'])
      p.wait()
      self.assertEqual(p.returncode, 1)

  @decorators.Enabled('chromeos')
  @mock.patch.object(cros_interface.CrOSInterface, 'RunCmdOnDevice')
  def testTryLoginSuccess(self, mock_run_cmd):
    mock_run_cmd.return_value = ('root\n', '')
    cri = cros_interface.CrOSInterface(
        "testhostname", 22, options_for_unittests.GetCopy().ssh_identity)
    cri.TryLogin()
    mock_run_cmd.assert_called_once_with(
        ['echo', '$USER'], quiet=True, connect_timeout=60)

  @decorators.Enabled('chromeos')
  @mock.patch.object(cros_interface.CrOSInterface, 'RunCmdOnDevice')
  def testTryLoginStderr(self, mock_run_cmd):
    cri = cros_interface.CrOSInterface(
        "testhostname", 22, options_for_unittests.GetCopy().ssh_identity)

    mock_run_cmd.return_value = ('', 'Host key verification failed')
    self.assertRaises(cros_interface.LoginException, cri.TryLogin)
    self.assertRaisesRegex(cros_interface.LoginException,
                            r'.*host key verification failed..*', cri.TryLogin)

    mock_run_cmd.return_value = ('', 'Operation timed out')
    self.assertRaisesRegex(cros_interface.LoginException,
                            r'Timed out while logging into.*', cri.TryLogin)

    mock_run_cmd.return_value = ('', 'UNPROTECTED PRIVATE KEY FILE!')
    self.assertRaisesRegex(cros_interface.LoginException,
                            r'Permissions for .* are too open. To fix this.*',
                            cri.TryLogin)

    mock_run_cmd.return_value = (
        '', 'Permission denied (publickey,keyboard-interactive)')
    self.assertRaisesRegex(cros_interface.KeylessLoginRequiredException,
                            r'Need to set up ssh auth for .*', cri.TryLogin)

    mock_run_cmd.return_value = ('', 'Fallback error case')
    self.assertRaisesRegex(cros_interface.LoginException,
                            r'While logging into .*, got .*', cri.TryLogin)

    mock_run_cmd.return_value = ('', 'Could not resolve hostname')
    self.assertRaisesRegex(cros_interface.DNSFailureException,
                            r'Unable to resolve the hostname for:.*',
                            cri.TryLogin)

  @decorators.Enabled('chromeos')
  @mock.patch.object(cros_interface.CrOSInterface, 'RunCmdOnDevice')
  def testTryLoginStdout(self, mock_run_cmd):
    mock_run_cmd.return_value = ('notrooot', '')
    cri = cros_interface.CrOSInterface(
        "testhostname", 22, options_for_unittests.GetCopy().ssh_identity)
    self.assertRaisesRegex(cros_interface.LoginException,
                            r'Logged into .*, expected \$USER=root, but got .*',
                            cri.TryLogin)

  @decorators.Enabled('chromeos')
  @mock.patch.object(cros_interface.CrOSInterface, 'RunCmdOnDevice')
  def testIsCryptohomeMounted(self, mock_run_cmd):
    # The device's mount state is checked by the command
    #   /bin/df --someoption `cryptohome-path user $username`.
    # The following mock replaces RunCmdOnDevice() to return mocked mount states
    # from the command execution.
    def mockRunCmdOnDevice(args): # pylint: disable=invalid-name
      if args[0] == 'cryptohome-path':
        return ('/home/user/%s' % args[2], '')
      if args[0] == 'nsenter':
        # 'nsenter' is used to find Guest sessions. Ignore it in unit tests.
        # 'nsenter' takes one argument so skip first two args.
        return mockRunCmdOnDevice(args[2:])
      if args[0] == '/bin/df':
        if 'unmount' in args[2]:
          # For the user unmount@gmail.com, returns the unmounted state.
          source, target = '/dev/sda1', '/home'
        elif 'ephemeral_mount' in args[2]:
          # For ephemeral mount, returns no mount.
          # TODO(poromov): Add test for ephemeral mount.
          return ('df %s: No such file or directory\n' % (args[2]), '')
        elif 'mount' in args[2]:
          # For the user mount@gmail.com, returns the mounted state.
          source, target = '/dev/sda1', args[2]
        elif 'guest' in args[2]:
          # For the user $guest, returns the guest-mounted state.
          source, target = 'guestfs', args[2]
        return ('Filesystem Mounted on\n%s %s\n' % (source, target), '')
      return 'Reached unreachable code'
    mock_run_cmd.side_effect = mockRunCmdOnDevice

    cri = cros_interface.CrOSInterface(
        "testhostname", 22, options_for_unittests.GetCopy().ssh_identity)
    # Returns False if the user's cryptohome is not mounted.
    self.assertFalse(cri.IsCryptohomeMounted('unmount@gmail.com', False))
    # Returns True if the user's cryptohome is mounted.
    self.assertTrue(cri.IsCryptohomeMounted('mount@gmail.com', False))
    # Returns True if the guest cryptohome is mounted.
    self.assertTrue(cri.IsCryptohomeMounted('$guest', True))
    # Sanity check. Returns False if the |is_guest| parameter does not match
    # with whether or not the user is really a guest.
    self.assertFalse(cri.IsCryptohomeMounted('unmount@gmail.com', True))
    self.assertFalse(cri.IsCryptohomeMounted('mount@gmail.com', True))
    self.assertFalse(cri.IsCryptohomeMounted('$guest', False))
