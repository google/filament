# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pylint: disable=too-many-lines
from __future__ import absolute_import
import builtins
import contextlib
import io
import os
import posixpath
import shutil
import subprocess
import tempfile
import time
import unittest
from unittest import mock
import random
import string

from devil.utils import cmd_helper
from telemetry.core import linux_based_interface
from telemetry import decorators
from telemetry.testing import options_for_unittests
from telemetry.util import cmd_util

class LinuxBasedInterfaceHelperMethodsTest(unittest.TestCase):

  @mock.patch.object(cmd_util, 'GetAllCmdOutput')
  @decorators.Enabled('linux', 'chromeos')
  def testGetAllCmdOutputCallsAndDecodes(self, mock_get_output):
    mock_stdout = mock.create_autospec(bytes, instance=True)
    mock_stdout.decode.return_value = 'decoded stdout'
    mock_stderr = mock.create_autospec(bytes, instance=True)
    mock_stderr.decode.return_value = 'decoded stderr'
    mock_get_output.return_value = (mock_stdout, mock_stderr)

    stdout, stderr = linux_based_interface.GetAllCmdOutput(['forward', 'me'])

    self.assertEqual(stdout, 'decoded stdout')
    self.assertEqual(stderr, 'decoded stderr')

    mock_stdout.decode.assert_called_once_with('utf-8')
    mock_stderr.decode.assert_called_once_with('utf-8')

    mock_get_output.assert_called_once_with(['forward', 'me'], None, False)

    mock_get_output.reset_mock()
    linux_based_interface.GetAllCmdOutput(['forward', 'me'],
                                          quiet=True,
                                          cwd='some/dir')
    mock_get_output.assert_called_once_with(['forward', 'me'], 'some/dir', True)

  @mock.patch.object(cmd_util, 'StartCmd')
  @decorators.Enabled('linux', 'chromeos')
  def testStartCmdForwardsArgs(self, mock_start):
    ret = mock.Mock()
    mock_start.return_value = ret

    self.assertEqual(linux_based_interface.StartCmd('cmd'), ret)

    mock_start.assert_called_once_with('cmd',
                                       cwd=None,
                                       quiet=False,
                                       stdout=None,
                                       stderr=None,
                                       env=None)

    mock_start.reset_mock()
    self.assertEqual(
        linux_based_interface.StartCmd(
            'another cmd', quiet=True, cwd='some/other/dir'), ret)
    mock_start.assert_called_once_with('another cmd', cwd='some/other/dir',
                                       quiet=True,
                                       stdout=None,
                                       stderr=None,
                                       env=None)

  @decorators.Enabled('linux', 'chromeos')
  def testUnquoteRemovesExternalQuotes(self):
    # For some reason, this use-case is supported.
    self.assertEqual(linux_based_interface._Unquote(0), 0)
    self.assertEqual(linux_based_interface._Unquote({}), {})
    self.assertEqual(linux_based_interface._Unquote([]), [])

    self.assertEqual(linux_based_interface._Unquote("'foo\""), "foo")
    self.assertEqual(linux_based_interface._Unquote('foo"""""'), "foo")
    self.assertEqual(linux_based_interface._Unquote("'''''''foo'\"'"), "foo")

    # This will even work for alternating quotes, no matter how many.
    self.assertEqual(linux_based_interface._Unquote("'\"'\"'\"foo'\"'"), "foo")


class LinuxBasedInterfaceTest(unittest.TestCase):

  def _GetLBI(self, remote=None, remote_ssh_port=None, ssh_identity=None):
    return linux_based_interface.LinuxBasedInterface(remote, remote_ssh_port,
                                                     ssh_identity)

  def _GetLocalLBI(self):
    return linux_based_interface.LinuxBasedInterface()

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @decorators.Enabled('linux', 'chromeos')
  def testLocal(self, unused_mock):
    with self._GetLocalLBI() as lbi:
      self.assertTrue(lbi.local)
      self.assertIsNone(lbi.hostname)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @decorators.Enabled('linux', 'chromeos')
  def testLocalIsFalseIfHostname(self, unused_mock):
    with self._GetLBI(remote='some address') as lbi:
      self.assertFalse(lbi.local)
      self.assertEqual(lbi.hostname, 'some address')

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @decorators.Enabled('linux', 'chromeos')
  def testCallsOpenConnectionOnInitRemote(self, mock_open):
    with self._GetLBI(remote='address'):
      mock_open.assert_called_once_with()

  @mock.patch.object(subprocess, 'call')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'FormSSHCommandLine')
  @decorators.Enabled('linux', 'chromeos')
  def testOpenConnection(self, mock_call, mock_form_ssh):
    with self._GetLocalLBI() as lbi:
      # Check that local will return immediately.
      self.assertTrue(lbi.local)
      lbi.OpenConnection()
      self.assertEqual(mock_call.call_count, 0)
      self.assertEqual(mock_form_ssh.call_count, 0)

    mock_call.reset_mock()
    mock_form_ssh.reset_mock()

    # Test remote case calls OpenConnection on start.
    with self._GetLBI(remote='address') as lbi:
      mock_form_ssh.assert_called_once()
      mock_call.assert_called_once()

      self.assertTrue(lbi._master_connection_open)

      # Assert no duplicate calls if already open.
      lbi.OpenConnection()
      self.assertEqual(mock_form_ssh.call_count, 1)
      self.assertEqual(mock_call.call_count, 1)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @decorators.Enabled('linux', 'chromeos')
  def testFormSSHCommandLineLocal(self, unused_mock):
    with self._GetLocalLBI() as lbi:
      self.assertEqual(
          lbi.FormSSHCommandLine(['some', 'args']), ['sh', '-c', 'some args'])

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @decorators.Enabled('linux', 'chromeos')
  def testFormSSHCommandLineRemote(self, unused_conn):
    with self._GetLBI(remote='address') as lbi:
      lbi._DEFAULT_SSH_CONNECTION_TIMEOUT = 42
      lbi._DEFAULT_SSH_COMMAND = ['fizz', 'buzz']
      lbi._ssh_args = ['-o SomeOptions', '-o ToAdd']
      lbi._ssh_control_file = 'some_file'
      lbi._ssh_port = 8222
      lbi._ssh_identity = 'foo-bar'

      result = lbi.FormSSHCommandLine(['some', 'args'])
      formed = ' '.join(result)

      # Ensure the correct components are present
      # SSH command
      self.assertIn('fizz buzz', formed)
      # Default timeout
      self.assertIn('-o ConnectTimeout=42', formed)
      # Control file (if no port-forwarding)
      self.assertIn('-S some_file', formed)
      # SSH Args.
      self.assertIn('-o SomeOptions -o ToAdd', formed)
      # Check root and SSH port
      self.assertIn('root@address', formed)
      self.assertIn('-p8222', formed)
      # SSH Identity
      self.assertIn('-i foo-bar', formed)
      # args given.
      self.assertIn('some args', formed)

      # Timeout override, extra_args, and port_forward
      result = lbi.FormSSHCommandLine(
          ['some', 'args'],
          connect_timeout=57,
          port_forward=True,
          extra_ssh_args=['additional', 'ssh', 'args'])
      formed = ' '.join(result)
      # Default timeout
      self.assertIn('-o ConnectTimeout=57', formed)
      # Control file not present if  port-forwarding
      self.assertNotIn('-S some_file', formed)
      # extra args given.
      self.assertIn('additional ssh args', formed)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @decorators.Enabled('linux', 'chromeos')
  def testRemoveSSHWarnings(self, unused_mock):
    with self._GetLBI() as lbi:
      filter_string = ('Warning: Permanently added something to '
                       'the list of known hosts.\n\n')
      test_string = filter_string + 'Output from some command'

      self.assertEqual(
          lbi._RemoveSSHWarnings(test_string), 'Output from some command')

      test_string = filter_string.replace(
          'something', 'another address with whitespace') + 'foo'
      self.assertEqual(lbi._RemoveSSHWarnings(test_string), 'foo')

      # Does not remove.
      test_string = filter_string.replace('something to', 'something') + 'foo'
      self.assertEqual(lbi._RemoveSSHWarnings(test_string), test_string)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'FormSSHCommandLine')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     '_RemoveSSHWarnings')
  @mock.patch.object(linux_based_interface, 'GetAllCmdOutput')
  @decorators.Enabled('linux', 'chromeos')
  def testRunCmdOnDeviceUsesArgsCorrectly(self, mock_get_output,
                                          mock_remove_warnings, mock_form_ssh,
                                          unused_mock):
    with self._GetLBI() as lbi:
      mock_form_ssh.return_value = ['some', 'args']
      mock_get_output.return_value = ('stdout', 'stderr')
      mock_remove_warnings.return_value = 'filtered stderr'

      stdout, stderr = lbi.RunCmdOnDevice(['additional', 'args'],
                                          cwd='/some/dir',
                                          quiet=True,
                                          connect_timeout=57,
                                          port_forward=True)
      self.assertEqual(stdout, 'stdout')
      self.assertEqual(stderr, 'filtered stderr')

      mock_form_ssh.assert_called_once_with(['additional', 'args'],
                                            connect_timeout=57,
                                            port_forward=True,
                                            env=None)
      mock_get_output.assert_called_once_with(['some', 'args'],
                                              cwd='/some/dir',
                                              quiet=True)
      mock_remove_warnings.asset_called_once_with('stderr')

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testTryLoginRunsEcho(self, mock_run_cmd, unused_mock):
    with self._GetLBI(remote='address') as lbi:
      lbi._REMOTE_USER = 'foo_user'
      mock_run_cmd.return_value = ('foo_user\n', '')
      lbi.TryLogin()

      mock_run_cmd.assert_called_once_with(['echo', '$USER'],
                                           quiet=mock.ANY,
                                           connect_timeout=mock.ANY)

      mock_run_cmd.return_value = 'bad user', ''
      self.assertRaises(linux_based_interface.LoginException, lbi.TryLogin)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testTryLoginErrors(self, mock_run_cmd, unused_mock):
    with self._GetLBI(remote='address') as lbi:
      mock_run_cmd.return_value = ('', 'random error')

      self.assertRaisesRegex(linux_based_interface.LoginException,
                             'logging into address, got random error',
                             lbi.TryLogin)

      mock_run_cmd.return_value = ('', lbi._HOST_KEY_ERROR)
      self.assertRaisesRegex(linux_based_interface.LoginException,
                             'address host key verification failed',
                             lbi.TryLogin)
      mock_run_cmd.return_value = ('', lbi._TIMEOUT_ERROR)
      self.assertRaisesRegex(linux_based_interface.LoginException,
                             '[tT]imed out', lbi.TryLogin)
      mock_run_cmd.return_value = ('', lbi._PRIV_KEY_PROTECTIONS_ERROR)
      self.assertRaisesRegex(linux_based_interface.LoginException,
                             '[pP]ermissions.*are too open', lbi.TryLogin)
      mock_run_cmd.return_value = ('', lbi._SSH_AUTH_ERROR)
      self.assertRaisesRegex(linux_based_interface.LoginException,
                             '[nN]eed to set up ssh auth', lbi.TryLogin)
      mock_run_cmd.return_value = ('', lbi._HOSTNAME_RESOLUTION_ERROR)
      self.assertRaisesRegex(linux_based_interface.DNSFailureException,
                             '[uU]nable to resolve the hostname for: address',
                             lbi.TryLogin)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice', return_value=('',''))
  @decorators.Enabled('linux', 'chromeos')
  def testTestFileAssertAnExistenceCondition(self, unused_mock_run,
                                             unused_mock):
    with self._GetLBI(remote='address') as lbi:
      self.assertRaises(AssertionError, lbi.TestFile, 'foo')
      self.assertRaises(AssertionError, lbi.TestFile, 'foo',
                        is_file=True, is_dir=True)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice', return_value=('', 'stderr'))
  @decorators.Enabled('linux', 'chromeos')
  def testTestFileThrowsErrorOnFailure(self, unused_mock_run, unused_mock):
    with self._GetLBI(remote='address') as lbi:
      self.assertRaises(OSError, lbi.TestFile, 'foo', exists=True)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  def testTestFileChecksResultCorrectly(self, mock_run, unused_mock):
    with self._GetLBI(remote='address') as lbi:
      mock_run.return_value = '1\n', ''
      self.assertTrue(lbi.TestFile('foo', exists=True))

      mock_run.return_value = 'Not 1\n', ''
      self.assertFalse(lbi.TestFile('foo', exists=True))

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testTestFileUsesCorrectConditions(self, mock_run, unused_mock):
    with self._GetLBI(remote='address') as lbi:
      mock_run.return_value = '1\n', ''
      self.assertTrue(lbi.TestFile('foo', exists=True))

      mock_run.assert_called_once_with(
          'if test -e foo ; then echo 1 ; fi'.split())
      mock_run.reset_mock()

      self.assertTrue(lbi.TestFile('foo', is_file=True))
      mock_run.assert_called_once_with(
          'if test -f foo ; then echo 1 ; fi'.split())
      mock_run.reset_mock()

      self.assertTrue(lbi.TestFile('foo', is_dir=True))
      mock_run.assert_called_once_with(
          'if test -d foo ; then echo 1 ; fi'.split())

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testFileExistsOnDevice(self, mock_run_cmd, unused_mock):
    with self._GetLBI(remote='address') as lbi:
      mock_run_cmd.return_value = ('1\n', '')
      self.assertTrue(lbi.FileExistsOnDevice('foo'))

      # Check that file existence is called.
      self.assertIn('test -e foo', ' '.join(mock_run_cmd.call_args[0][0]))

      # Check bad stdout returns False.
      mock_run_cmd.return_value = ('', '')
      self.assertFalse(lbi.FileExistsOnDevice('foo'))

      # Check that errors are raised if stderr.
      mock_run_cmd.return_value = ('1\n', 'some stderr boohoo')
      self.assertRaisesRegex(OSError, '[uU]nexpected error: some stderr boohoo',
                             lbi.FileExistsOnDevice, 'foo')

      # Specific failure case.
      mock_run_cmd.return_value = ('1\n', 'Connection timed out')
      self.assertRaisesRegex(OSError, 'wasn\'t responding to ssh',
                             lbi.FileExistsOnDevice, 'foo')

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface, 'GetAllCmdOutput')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     '_FormSCPToRemote')
  @mock.patch.object(os.path, 'exists', return_value=True)
  @decorators.Enabled('linux', 'chromeos')
  def testPushFileLocal(self, unused_mock_exists, mock_form_scp, mock_run_cmd,
                        unused_mock):
    with self._GetLocalLBI() as lbi:
      self.assertTrue(lbi.local)
      mock_run_cmd.return_value = ('no error but who cares about stdout', '')

      lbi.PushFile('foo', 'remote_foo')

      self.assertRegex(' '.join(mock_run_cmd.call_args[0][0]),
                       r'cp.*foo\sremote_foo')
      self.assertEqual(mock_form_scp.call_count, 0)

      mock_run_cmd.return_value = ('error in stderr!', 'stderr')

      self.assertRaises(OSError, lbi.PushFile, 'foo', 'remote_foo')

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface, 'GetAllCmdOutput')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     '_FormSCPToRemote')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     '_RemoveSSHWarnings')
  @mock.patch.object(os.path, 'abspath')
  @mock.patch.object(os.path, 'exists')
  @decorators.Enabled('linux', 'chromeos')
  def testPushFileRemote(self, mock_exists,
                         mock_abspath, mock_ssh_warnings, mock_form_scp,
                         mock_run_cmd, unused_mock):
    mock_exists.side_effect = [
        False,  # To ignore ssh_control_file check
        True, True]
    with self._GetLBI(remote='address') as lbi:
      self.assertFalse(lbi.local)
      mock_run_cmd.return_value = ('stdout do not care',
                                   'stderr but will get filtered')
      mock_abspath.return_value = '/some/abs/path/to/foo'
      mock_form_scp.return_value = 'the scp cmd'.split(' ')
      mock_ssh_warnings.return_value = ''

      lbi.PushFile('foo', 'remote_foo')

      # abspath is called in ctor
      mock_abspath.assert_has_calls([mock.call('foo')])
      mock_form_scp.assert_called_once_with(
          '/some/abs/path/to/foo', 'remote_foo', extra_scp_args=mock.ANY,
      user='root')
      mock_run_cmd.assert_called_once_with(
          'the scp cmd'.split(' '), quiet=mock.ANY)
      mock_ssh_warnings.assert_called_once_with('stderr but will get filtered')

      mock_ssh_warnings.return_value = 'error in stderr!'

      self.assertRaises(OSError, lbi.PushFile, 'foo', 'remote_foo')

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface, 'PushFile')
  @mock.patch.object(tempfile, 'NamedTemporaryFile')
  @decorators.Enabled('linux', 'chromeos')
  def testPushContents(self, mock_tempfile, mock_push_file, unused_mock):
    with self._GetLocalLBI() as lbi:

      class NamedStringIO(io.StringIO):

        def __init__(self, name):
          super().__init__()
          self.name = name

      output = NamedStringIO(name='the_temp_file_name')

      mock_tempfile.return_value.__enter__.return_value = output

      lbi.PushContents('foo contents', 'remote_file')
      # assert writeable.
      self.assertIn('w', mock_tempfile.call_args[1]['mode'])
      self.assertEqual(output.getvalue(), 'foo contents')
      mock_push_file.assert_called_once_with(output.name, 'remote_file')

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     '_FormSCPFromRemote')
  @mock.patch.object(linux_based_interface, 'GetAllCmdOutput')
  @mock.patch.object(os.path, 'abspath')
  @mock.patch.object(shutil, 'copyfile')
  @decorators.Enabled('linux', 'chromeos')
  def testGetFileLocal(self, mock_copyfile, mock_abspath, mock_cmd, mock_scp,
                       unused_mock):
    with self._GetLocalLBI() as lbi:
      self.assertTrue(lbi.local)
      mock_abspath.return_value = '/local/file/abspath'

      lbi.GetFile('remote/file', 'local/file')

      # Ensure there is no SCP'ing in local case.
      self.assertEqual(mock_scp.call_count, 0)
      self.assertEqual(mock_cmd.call_count, 0)

      # Verify copy is called.
      mock_abspath.assert_called_once_with('local/file')
      mock_copyfile.assert_called_once_with('remote/file',
                                            '/local/file/abspath')

      mock_copyfile.reset_mock()
      mock_abspath.reset_mock()

      mock_abspath.return_value = '/identical/file'
      # Assert raises error if identical files calling copy.
      self.assertRaises(OSError, lbi.GetFile, '/identical/file',
                        '/identical/file')

      mock_copyfile.reset_mock()
      mock_abspath.reset_mock()
      mock_abspath.return_value = '/local/file/dir'

      # Copying from one dir to local dir/relative dir should be possible.
      lbi.GetFile('/path/in/other/dir')

      # basename is used to call the abspath.
      mock_abspath.assert_called_once_with('dir')
      mock_copyfile.assert_called_once_with('/path/in/other/dir',
                                            '/local/file/dir')

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     '_FormSCPFromRemote')
  @mock.patch.object(linux_based_interface, 'GetAllCmdOutput')
  @mock.patch.object(os.path, 'abspath')
  @mock.patch.object(shutil, 'copyfile')
  @decorators.Enabled('linux', 'chromeos')
  def testGetFileRemote(self, mock_copyfile, mock_abspath, mock_cmd, mock_scp,
                        unused_mock):
    with self._GetLBI(remote='address') as lbi:
      self.assertFalse(lbi.local)
      mock_abspath.return_value = '/local/file/abspath'
      lbi._disable_strict_filenames = False
      mock_scp.return_value = 'some scp command'.split(' ')
      mock_cmd.return_value = ('no stderr', '')

      lbi.GetFile('remote/file', 'local/file')

      # Ensure there is no SCP'ing in local case.
      self.assertEqual(mock_copyfile.call_count, 0)

      mock_scp.assert_called_once_with(
          'remote/file', '/local/file/abspath', extra_scp_args=[],
          user='root')
      mock_cmd.assert_called_once_with(
          'some scp command'.split(' '), quiet=mock.ANY)

      mock_cmd.return_value = ('stderr incoming!', 'stderr')
      self.assertRaises(OSError, lbi.GetFile, 'does it matter', 'at all')

      mock_cmd.reset_mock()
      mock_scp.reset_mock()

      # Check that edge case with filename mismatch is dealt with.
      mock_cmd.side_effect = [('stderr incoming!',
                               'filename does not match request'),
                              ('stderr works!', '')]
      mock_scp.side_effect = [
          'first scp command'.split(' '), 'second scp command'.split(' ')
      ]

      self.assertFalse(lbi._disable_strict_filenames)
      lbi.GetFile('remote/file', 'local/file')
      self.assertTrue(lbi._disable_strict_filenames)

      mock_scp.assert_has_calls([
          mock.call('remote/file', '/local/file/abspath', extra_scp_args=[],
                    user='root'),
          mock.call(
              'remote/file', '/local/file/abspath', extra_scp_args=['-T'],
          user='root')
      ])
      mock_cmd.assert_has_calls([
          mock.call('first scp command'.split(' '), quiet=mock.ANY),
          mock.call('second scp command'.split(' '), quiet=mock.ANY),
      ])

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface, 'GetFile')
  @mock.patch.object(tempfile, 'NamedTemporaryFile')
  @mock.patch.object(builtins, 'open')
  @decorators.Enabled('linux', 'chromeos')
  def testGetFileContents(self, mock_open, mock_named_temp, mock_getfile,
                          unused_mock):
    with self._GetLBI(remote='address') as lbi:
      some_mock = mock.Mock()
      some_mock.name = 'foo'
      mock_named_temp.return_value.__enter__.return_value = some_mock

      fake_file = io.StringIO('these are file contents')

      mock_open.return_value.__enter__.return_value = fake_file

      self.assertEqual(
          lbi.GetFileContents('path/to/some/file'), 'these are file contents')

      # Check that we got a temp file created
      mock_named_temp.assert_called_once_with(mode='w')
      mock_getfile.assert_called_once_with('path/to/some/file', 'foo')
      # Check that it was opened for reading.
      mock_open.assert_called_once_with('foo', 'r', encoding=mock.ANY)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @mock.patch.object(time, 'time')
  @decorators.Enabled('linux', 'chromeos')
  def testGetDeviceHostClockOffset(self, mock_time, mock_run_cmd, unused_mock):
    with self._GetLocalLBI() as lbi:
      self.assertIsNone(lbi._device_host_clock_offset)
      # Device time. Expects string.
      mock_run_cmd.return_value = '1053', 'ignored stderr'

      # Host time.
      mock_time.return_value = 1000

      self.assertEqual(lbi.GetDeviceHostClockOffset(), 53)

      # Offset is now cached.
      self.assertEqual(lbi._device_host_clock_offset, 53)

      # Offset cannot be updated. If it did, the offset would be 0.
      mock_time.return_value = 1053
      self.assertEqual(lbi.GetDeviceHostClockOffset(), 53)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testHasSystemD(self, mock_cmd, unused_mock):
    with self._GetLocalLBI() as lbi:
      mock_cmd.return_value = 'ignored', ''

      self.assertTrue(lbi.HasSystemd())

      mock_cmd.assert_called_once_with(['systemctl'], quiet=mock.ANY)

      mock_cmd.reset_mock()
      mock_cmd.return_value = 'ignored', 'some error'

      self.assertFalse(lbi.HasSystemd())

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testListProcesses(self, mock_cmd, unused_mock):
    ps_output = """
    1       0 /sbin/init splash               S
    2       0 [kthreadd]                      S
    3       2 [rcu_gp]                        I
    4       2 [rcu_par_gp]                    I
    5       2 [netns]                         I
    7       2 [kworker/0:0H-events_highpri]   I
"""
    expected_procs_parsed = [
        (1, '/sbin/init splash', 0, 'S'),
        (2, '[kthreadd]', 0, 'S'),
        (3, '[rcu_gp]', 2, 'I'),
        (4, '[rcu_par_gp]', 2, 'I'),
        (5, '[netns]', 2, 'I'),
        (7, '[kworker/0:0H-events_highpri]', 2, 'I'),
    ]
    with self._GetLocalLBI() as lbi:
      mock_cmd.return_value = ps_output, ''

      self.assertEqual(lbi.ListProcesses(), expected_procs_parsed)
      self.assertIn('/bin/ps', mock_cmd.call_args[0][0])

      mock_cmd.return_value = ps_output, 'stderr'
      self.assertRaises(AssertionError, lbi.ListProcesses)

      mock_cmd.return_value = ps_output + '\nThis line will cause issues', ''
      self.assertRaises(AssertionError, lbi.ListProcesses)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface, 'ListProcesses')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testKillAllMatching(self, mock_run_cmd, mock_list, unused_mock):
    with self._GetLocalLBI() as lbi:
      mock_list.return_value = [
          (0, 'some cmd', 'unused', 'unused'),
          (1, 'some other cmd', 'unused', 'unused'),
          (2, 'some new cmd', 'unused', 'unused'),
          (3, 'cmd', 'unused', 'unused'),
      ]

      removed = []
      count = 0

      def predicate_kill_every_other(cmd):
        del cmd
        nonlocal count
        if count % 2:
          count += 1
          return True
        count += 1
        return False

      predicate_kill_all = lambda a: True
      predicate_kill_none = lambda a: False

      self.assertEqual(lbi.KillAllMatching(predicate_kill_none), 0)
      self.assertEqual(mock_run_cmd.call_count, 0)

      mock_run_cmd.reset_mock()
      self.assertEqual(lbi.KillAllMatching(predicate_kill_all), 4)
      mock_run_cmd.assert_called_once_with(['kill', '-KILL', '0', '1', '2',
                                            '3'],
                                           quiet=mock.ANY)

      mock_run_cmd.reset_mock()
      num_kills = lbi.KillAllMatching(predicate_kill_every_other)
      removed = ['1', '3']
      self.assertEqual(num_kills, len(removed))
      mock_run_cmd.assert_called_once_with(
          ['kill', '-KILL'] + removed, quiet=mock.ANY)

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface, 'HasSystemd')
  @decorators.Enabled('linux', 'chromeos')
  def testIsServiceRunning(self, mock_systemd, mock_cmd, unused_mock):
    with self._GetLocalLBI() as lbi:
      # Test default case.
      mock_systemd.return_value = False

      self.assertRaisesRegex(AssertionError, 'system does not have systemd',
                             lbi.IsServiceRunning, 'foo-service')

      mock_systemd.assert_called_once_with()
      self.assertEqual(mock_cmd.call_count, 0)
      mock_systemd.reset_mock()

      # Test stderr from cmd will throw error.
      mock_systemd.return_value = True
      mock_cmd.return_value = 'MainPID=0', 'some stderr'
      self.assertRaisesRegex(AssertionError, 'some stderr',
                             lbi.IsServiceRunning, 'foo-service')
      mock_cmd.assert_called_once_with(
          ['systemctl', 'show', '-p', 'MainPID', 'foo-service'], quiet=mock.ANY)

      # Test PID=0 returns false
      mock_cmd.return_value = 'MainPID=0', ''
      self.assertFalse(lbi.IsServiceRunning('foo-service'))

      # Test PID!=0 returns True
      mock_cmd.return_value = 'MainPID=53', ''
      self.assertTrue(lbi.IsServiceRunning('foo-service'))

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testGetRemotePort(self, mock_cmd, unused_mock):
    netstat_output = ("""Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State
tcp        0      0 127.0.0.1:19080         0.0.0.0:*               LISTEN
tcp        0      0 127.0.0.2:6697          0.0.0.0:*               LISTEN
tcp        0      0 127.0.0.1:9557          0.0.0.0:*               LISTEN
""", 'ignored stderr')

    with self._GetLocalLBI() as lbi:
      lbi._reserved_ports = []
      mock_cmd.return_value = netstat_output

      # Port given will be largest port in use +1
      # This includes previously reserved ports
      self.assertEqual(lbi.GetRemotePort(), 19081)
      self.assertEqual(lbi._reserved_ports, [19081])
      mock_cmd.assert_called_once_with(['netstat', '-ant'])

      self.assertEqual(lbi.GetRemotePort(), 19082)

      lbi._reserved_ports.append(30000)
      self.assertEqual(lbi.GetRemotePort(), 30001)
      self.assertEqual(lbi._reserved_ports, [19081, 19082, 30000, 30001])

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testIsHTTPServerRunningOnPort(self, mock_cmd, unused_mock):
    with self._GetLocalLBI() as lbi:
      mock_cmd.return_value = 'some stdout from wget', 'Connection refused'

      self.assertFalse(lbi.IsHTTPServerRunningOnPort(5000))

      # Don't care about timeout period being there.
      args = mock_cmd.call_args[0][0]
      self.assertIn('wget', args)
      self.assertIn('localhost:5000', args)

      mock_cmd.reset_mock()
      mock_cmd.return_value = ('Connection somewhere not refused',
                               'Connection NOT refused')

      self.assertTrue(lbi.IsHTTPServerRunningOnPort(8008))

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'CanTakeScreenshot', return_value=True)
  @mock.patch.object(linux_based_interface.LinuxBasedInterface, 'GetFile')
  @decorators.Enabled('linux', 'chromeos')
  def testTakeScreenshotFailsIfBadStdout(
      self,
      unused_mock_get_file,
      unused_mock_take_screenshot,
      mock_run_cmd,
      unused_mock,
  ):
    with self._GetLocalLBI() as lbi:
      self.assertTrue(lbi.local)
      mock_run_cmd.side_effect = [
          ('making a directory', ''),
          # This line causes the failure => value!=0
          ('screenshot return value:25', ''),
          ('making a directory', ''),
          ('copying a file', ''),
      ]

      self.assertFalse(lbi.TakeScreenshot('/path/to/foo'))

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'CanTakeScreenshot', return_value=True)
  @mock.patch.object(linux_based_interface.LinuxBasedInterface, 'GetFile')
  @decorators.Enabled('linux', 'chromeos')
  def testTakeScreenshotLocallyMakesDirectoryAndCopiesFile(
      self,
      unused_mock_get_file,
      unused_mock_can_take_screenshot,
      mock_run_cmd,
      unused_mock,
  ):
    with self._GetLocalLBI() as lbi:
      self.assertTrue(lbi.local)
      mock_run_cmd.side_effect = [
          ('making a directory', ''),
          ('screenshot return value:0', ''),
          ('making a directory', ''),
          ('copying a file', ''),
      ]

      self.assertTrue(lbi.TakeScreenshot('/path/to/foo'))

      mock_run_cmd.assert_has_calls([
          mock.call('mkdir -p /tmp/log/screenshots'.split(' ')),
          mock.call((lbi._SCREENSHOT_BINARY +
                     ' -f /tmp/log/screenshots/foo && echo').split(' ') +
                    ['screenshot return value:$?']),
          mock.call('mkdir -p /path/to'.split(' ')),
          mock.call('cp /tmp/log/screenshots/foo /path/to/foo'.split(' '))
      ])

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'CanTakeScreenshot', return_value=True)
  @mock.patch.object(linux_based_interface.LinuxBasedInterface, 'GetFile')
  @mock.patch.object(os.path, 'exists')
  @mock.patch.object(os, 'makedirs')
  @decorators.Enabled('linux', 'chromeos')
  def testTakeScreenshotRemoteMakesDirectoryAndCopiesFile(
      self, mock_mkdirs, mock_exists, mock_get_file,
      unused_mock_take_screenshot, mock_run_cmd, unused_mock):
    for dir_exists_case in [True, False]:
      with self.subTest(dir_exists=dir_exists_case):
        mock_mkdirs.reset_mock()
        mock_get_file.reset_mock()
        mock_exists.reset_mock()
        mock_run_cmd.reset_mock()
        mock_exists.side_effect = [False, dir_exists_case]
        with self._GetLBI(remote='address') as lbi:
          self.assertFalse(lbi.local)
          mock_run_cmd.side_effect = [
              ('making a directory', ''),
              ('screenshot return value:0', ''),
          ]
          mock_exists.return_value = dir_exists_case

          self.assertTrue(lbi.TakeScreenshot('/path/to/foo'))

          mock_run_cmd.assert_has_calls([
              mock.call('mkdir -p /tmp/log/screenshots'.split(' ')),
              mock.call((lbi._SCREENSHOT_BINARY +
                         ' -f /tmp/log/screenshots/foo && echo').split(' ') +
                        ['screenshot return value:$?']),
          ])
          # Check that it created the directory if it didn't exist.
          mock_exists.assert_has_calls([mock.call('/path/to')])
          if dir_exists_case:
            mock_mkdirs.assert_not_called()
          else:
            mock_mkdirs.assert_called_once_with('/path/to')
          mock_get_file.assert_called_once_with('/tmp/log/screenshots/foo',
                                                '/path/to/foo')

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'TakeScreenshot')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'FileExistsOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testTakeScreenshotWithPrefixSucceeds(self, mock_file_exists, mock_run_cmd,
                                           mock_screenshot, unused_mock):
    with self._GetLocalLBI() as lbi:
      mock_file_exists.return_value = False

      self.assertTrue(lbi.TakeScreenshotWithPrefix('test-prefix'))

      screenshot_file = '/tmp/telemetry/screenshots/test-prefix-0.png'
      mock_file_exists.assert_called_once_with(screenshot_file)
      mock_screenshot.assert_called_once_with(screenshot_file)
      mock_run_cmd.assert_called_once_with(
          ['mkdir', '-p', '/tmp/telemetry/screenshots/'])

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'TakeScreenshot')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'FileExistsOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testTakeScreenshotWithPrefixFailsIfFull(self, mock_file_exists,
                                              mock_run_cmd, mock_screenshot,
                                              unused_mock):
    with self._GetLocalLBI() as lbi:
      # Simulate as if there were too many screenshots.
      mock_file_exists.side_effect = [True, True]

      self.assertFalse(lbi.TakeScreenshotWithPrefix('test-prefix'))

      mock_run_cmd.assert_called_once_with(
          ['mkdir', '-p', '/tmp/telemetry/screenshots/'])

      # Assert there were no screenshots - disc is "full"
      self.assertEqual(mock_screenshot.call_count, 0)

      screenshot_file = '/tmp/telemetry/screenshots/test-prefix-%d.png'
      mock_file_exists.assert_has_calls(
          [mock.call(screenshot_file % 0),
           mock.call(screenshot_file % 1)])

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @decorators.Enabled('linux', 'chromeos')
  def testGetArchName(self, mock_cmd, unused_mock):
    with self._GetLocalLBI() as lbi:
      mock_cmd.return_value = 'foo-arch\n', 'ignored stderr'
      self.assertEqual(lbi.GetArchName(), 'foo-arch')
      mock_cmd.assert_called_once_with(['uname', '-m'])

      # Check that its cached.
      self.assertIsNotNone(lbi._arch_name)
      mock_cmd.return_value = 'buzz-arch\n', 'ignored stderr'
      self.assertEqual(lbi.GetArchName(), 'foo-arch')
      mock_cmd.assert_called_once_with(['uname', '-m'])

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'GetFileContents')
  @decorators.Enabled('linux', 'chromeos')
  def testLsbReleaseValue(self, mock_contents, unused_mock):
    example_contents = """DISTRIB_CODENAME=buzzian
DISTRIB_DESCRIPTION="Debian GNU/Linux buzzian"
DISTRIB_ID=Debian
DISTRIB_RELEASE=buzzian
ANOTHER_PROPERTY=foobuntu
"""
    with self._GetLocalLBI() as lbi:
      mock_contents.return_value = example_contents
      self.assertEqual(
          lbi.LsbReleaseValue('DISTRIB_DESCRIPTION', 'default'),
          '"Debian GNU/Linux buzzian"')
      mock_contents.assert_called_once_with('/etc/lsb-release')
      self.assertEqual(
          lbi.LsbReleaseValue('DISTRIB_CODENAME', 'default'), 'buzzian')
      self.assertEqual(lbi.LsbReleaseValue('DISTRIB_ID', 'default'), 'Debian')
      self.assertEqual(
          lbi.LsbReleaseValue('DISTRIB_RELEASE', 'default'), 'buzzian')
      self.assertEqual(
          lbi.LsbReleaseValue('ANOTHER_PROPERTY', 'default'), 'foobuntu')
      self.assertEqual(
          lbi.LsbReleaseValue('DNE_PROPERTY', 'default'), 'default')

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'CloseConnection')
  @decorators.Enabled('linux', 'chromeos')
  def testOpenAndCloseAreCalled(self, mock_close, mock_open):
    with self._GetLBI(remote='address'):
      mock_open.assert_called_once_with()
      self.assertEqual(mock_close.call_count, 0)

    mock_open.assert_called_once_with()
    mock_close.assert_called_once_with()

  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'OpenConnection')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'RunCmdOnDevice')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'GetDeviceHostClockOffset')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'IsDir')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'FileExistsOnDevice')
  @mock.patch.object(linux_based_interface.LinuxBasedInterface,
                     'GetFile')
  @mock.patch.object(os, 'utime')
  @mock.patch.object(os.path, 'exists')
  @decorators.Enabled('linux', 'chromeos')
  def testPullDumps(
      self, mock_local_exists, mock_utime, mock_get_file, mock_file_exists,
      mock_is_dir, mock_offset, mock_cmd, unused_mock_open):
    mock_local_exists.side_effect = lambda x: 'path_exists_on_host' in x
    with self._GetLBI(remote='address') as lbi:
      # Time offset used later.
      mock_offset.return_value = 100
      mock_file_exists.side_effect = lambda x: 'bar.lock' in x
      mock_is_dir.side_effect = lambda x: 'this_will_be_a_dir' in x

      mock_cmd.side_effect = [
          ('root', ''),
          ('\n'.join((
              'foo.dat',  # Ignored extension
              'bar.lock',  # Ignored extension
              'bar',  # Ignored b/c lock exists for it.
              'this_will_be_a_dir',  # Will return True is on IsDir
              'path_exists_on_host',  # Will exist on host
              'actual_good_file_to_pull',  # Will get copied
          )), ''),
          # Additional calls for each file.
          ('-rw-r----- 1 fizz_buzz primarygroup    0 4200 __init__.py', ''),
          ('-rw-r----- 1 fizz_buzz primarygroup    0 4200 __init__.py', ''),
          ('-rw-r----- 1 fizz_buzz primarygroup    0 4200 __init__.py', ''),
          ('-rw-r----- 1 fizz_buzz primarygroup    0 4200 __init__.py', ''),
          ('-rw-r----- 1 fizz_buzz primarygroup    0 4200 __init__.py', ''),
          ('-rw-r----- 1 fizz_buzz primarygroup    0 4200 __init__.py', ''),
          ('-rw-r----- 1 fizz_buzz primarygroup    0 4200 __init__.py', ''),
      ]


      original_dir = os.path.join('tmp', 'foo', 'bar')
      lbi.PullDumps(original_dir)

      mock_get_file.assert_called_once_with(
          cmd_helper.SingleQuote(posixpath.join(lbi.MINIDUMP_DIR,
                                                'actual_good_file_to_pull')),
          os.path.join(original_dir, 'actual_good_file_to_pull'))

      mock_utime.assert_called_once_with(
          os.path.join(original_dir, 'actual_good_file_to_pull'),
          (4200-100, 4200-100))


_GLOBAL_LBI = None

class LinuxBasedInterfaceIntegrationTest(unittest.TestCase):
  """Tests linux-based interface functionality on local and remote platforms."""

  @classmethod
  def _GetLBI(cls, remote=None, remote_ssh_port=None, ssh_identity=None):
    # pylint: disable=global-statement
    global _GLOBAL_LBI
    # pylint: enable=global-statement
    if _GLOBAL_LBI:
      return _GLOBAL_LBI

    remote = remote or options_for_unittests.GetCopy().remote
    remote_ssh_port = remote_ssh_port or options_for_unittests.GetCopy(
    ).remote_ssh_port
    ssh_identity = ssh_identity or options_for_unittests.GetCopy(
    ).ssh_identity

    _GLOBAL_LBI = linux_based_interface.LinuxBasedInterface(remote,
                                                            remote_ssh_port,
                                                            ssh_identity)
    return _GLOBAL_LBI

  @classmethod
  def setUpClass(cls):
    cls.lbi = cls._GetLBI()
    cls.tmp_test_dir = None

  @classmethod
  def tearDownClass(cls):
    cls.lbi.CloseConnection()

  @decorators.Enabled('linux', 'chromeos')
  def testRunCmdRunsBasicCommands(self):
    self.assertEqual(0, linux_based_interface.RunCmd(['echo', '"Hello World"']))

    with tempfile.TemporaryDirectory() as dir_:
      tmp_file = os.path.join(dir_, 'foo')
      self.assertFalse(os.path.isfile(tmp_file))
      # File does not exist, so this should fail.
      self.assertNotEqual(0, linux_based_interface.RunCmd(['ls', tmp_file]))

      self.assertEqual(0, linux_based_interface.RunCmd(['touch', tmp_file]))
      self.assertEqual(0, linux_based_interface.RunCmd(['ls', tmp_file]))

      self.assertTrue(os.path.isfile(tmp_file))

  @decorators.Enabled('linux', 'chromeos')
  def testRunCmdRunsChangesDir(self):
    with tempfile.TemporaryDirectory() as dir_:
      tmp_file = os.path.join(dir_, 'foo')
      self.assertFalse(os.path.isfile(tmp_file))
      self.assertEqual(0, linux_based_interface.RunCmd(['touch', tmp_file]))

      # Not in the directory.
      self.assertNotEqual(0, linux_based_interface.RunCmd(['ls', 'foo']))

      # In this directory.
      self.assertEqual(0, linux_based_interface.RunCmd(['ls', 'foo'], cwd=dir_))


  @contextlib.contextmanager
  def _PopulateTmpFileWithCommands(self, commands):
    with tempfile.NamedTemporaryFile(mode='w') as tmp_file:
      tmp_file.write('\n'.join(commands))
      tmp_file.flush()
      yield tmp_file


  @decorators.Enabled('linux', 'chromeos')
  def testGetAllCmdOutputGetsStdoutAndStderr(self):
    self.assertEqual(('Hello world!\n', ''),
                     linux_based_interface.GetAllCmdOutput(
                         ['echo', 'Hello world!']))

    # Check stderr can be extracted.
    cmds = ['echo output to stdout', 'echo output to stderr >> /dev/stderr']
    with self._PopulateTmpFileWithCommands(cmds) as tmp_file:
      self.assertEqual(('output to stdout\n', 'output to stderr\n'),
                     linux_based_interface.GetAllCmdOutput(['bash',
                                                            tmp_file.name]))

  @decorators.Enabled('linux', 'chromeos')
  def testGetAllCmdOutputChangesDirectories(self):
    # Check stderr can be extracted.
    cmds = ['echo output to stdout', 'echo output to stderr >> /dev/stderr']
    with self._PopulateTmpFileWithCommands(cmds) as tmp_file:
      tmp_file_dir, tmp_file_base = os.path.split(tmp_file.name)

      self.assertNotEqual(0, ['ls', tmp_file_base])
      self.assertEqual(('output to stdout\n', 'output to stderr\n'),
                     linux_based_interface.GetAllCmdOutput(['bash',
                                                            tmp_file_base],
                                                           cwd=tmp_file_dir))

  @decorators.Enabled('linux', 'chromeos')
  def testStartCmdExecutesAsyncCommand(self):
    proc = linux_based_interface.StartCmd(['echo', 'hello world'])
    proc.wait()
    self.assertEqual(proc.poll(), 0)

    # Capture output.
    cmds = ['echo output to stdout', 'echo output to stderr >> /dev/stderr']
    with self._PopulateTmpFileWithCommands(cmds) as tmp_file:
      proc = linux_based_interface.StartCmd(
          ['bash', tmp_file.name], stdout=subprocess.PIPE,
          stderr=subprocess.PIPE)
      proc.wait()
      self.assertEqual(proc.poll(), 0)
      stdout, stderr = proc.communicate()
      self.assertEqual(stdout.decode().strip(), 'output to stdout')
      self.assertEqual(stderr.decode().strip(), 'output to stderr')


  @decorators.Enabled('linux', 'chromeos')
  def testTryLoginSucceeds(self):
    if self.lbi.local:
      raise unittest.SkipTest('Cannot run TryLogin on local DUT')
    self.assertFalse(self.lbi.local)
    self.lbi.TryLogin()

  def _exists(self, dir_):
    rc, _, _ = self.lbi.RunCmdOnDeviceWithRC(['ls', dir_])
    return rc == 0

  @decorators.Enabled('linux', 'chromeos')
  def testMkdTempWithPrefix(self):
    dir_ = self.lbi.MkdTemp(prefix='/tmp/foo_XXXXXXX')

    self.assertIn('/tmp/foo', dir_[:len('/tmp/foo')])

    self.assertTrue(self._exists(dir_))

    # Delete.
    rc, _, _ = self.lbi.RunCmdOnDeviceWithRC(['rm', '-rf', dir_])
    self.assertEqual(rc, 0)

    self.assertFalse(self._exists(dir_))

  def _createTempFile(self, file_name, touch=False):
    if not self.tmp_test_dir or not self._exists(self.tmp_test_dir):
      self.tmp_test_dir = self.lbi.MkdTemp(prefix='/tmp/test_run_XXXXXX')

    test_file = self.lbi.path.join(self.tmp_test_dir, file_name)

    if touch:
      self.lbi.RunCmdOnDevice(['touch', test_file])
      self.assertTrue(self.lbi.IsFile(test_file))
    elif self.lbi.IsFile(test_file):
      self.lbi.RmRF(test_file)

    return test_file

  @decorators.Enabled('linux', 'chromeos')
  def testPushContentsPushesContentsToFile(self):
    test_file = self._createTempFile('bar.txt')
    self.assertFalse(self._exists(test_file))

    self.lbi.PushContents('some file contents\nanother line',
                     test_file)
    self.assertTrue(self._exists(test_file))

    # Verify contents are correct.
    stdout, stderr = self.lbi.RunCmdOnDevice(['cat', test_file])
    self.assertEqual(stdout, 'some file contents\nanother line')
    self.assertEqual(stderr, '')

  @decorators.Enabled('linux', 'chromeos')
  def testPushFileFailsIfFileDoesNotExistLocally(self):
    test_file = self._createTempFile('bar.txt')
    self.assertFalse(self._exists(test_file))
    local_path = 'some/local/file.txt'
    self.assertFalse(os.path.exists(local_path))

    self.assertRaises(AssertionError, self.lbi.PushFile, local_path, test_file)

  @decorators.Enabled('linux', 'chromeos')
  def testPushFileFailsIfDestinationDoesNotExist(self):
    local_path = 'file.txt'
    with open(local_path, 'w') as f:
      f.write('hello')

    remote_path = 'fake/path/will/not/exist'

    self.assertRaises(OSError, self.lbi.PushFile, local_path,
                      remote_path)

  @decorators.Enabled('linux', 'chromeos')
  def testGetFileContentsReturnsFileContents(self):
    test_file = self._createTempFile('foo.txt')
    self.assertFalse(self._exists(test_file))
    self.lbi.PushContents('some file contents\nanother line',
                     test_file)

    contents = self.lbi.GetFileContents(test_file)
    self.assertEqual(contents, 'some file contents\nanother line')

  @decorators.Enabled('linux', 'chromeos')
  def testGetFileCopiesFileToLocalDestFile(self):
    test_file = self._createTempFile('foo.txt')
    self.lbi.PushContents('random file contents', test_file)

    with tempfile.TemporaryDirectory() as dir_:
      dest_file = os.path.join(dir_, 'foo.txt')
      self.lbi.GetFile(test_file, dest_file)

      with open(dest_file, 'r') as f:
        self.assertEqual(f.read(), 'random file contents')

  @decorators.Enabled('linux', 'chromeos')
  def testTestFileUtils(self):
    path = self._createTempFile('foo', touch=True)
    self.assertTrue(self._exists(path))

    self.assertTrue(self.lbi.TestFile(path, exists=True))
    self.assertTrue(self.lbi.FileExistsOnDevice(path))

    self.assertTrue(self.lbi.TestFile(path, is_file=True))
    self.assertTrue(self.lbi.IsFile(path))

    # Delete file.
    self.lbi.RmRF(path)
    self.assertFalse(self._exists(path))
    self.assertFalse(self.lbi.TestFile(path, exists=True))
    self.assertFalse(self.lbi.FileExistsOnDevice(path))

    self.assertFalse(self.lbi.TestFile(path, is_file=True))
    self.assertFalse(self.lbi.IsFile(path))

    dir_name = self.lbi.path.dirname(path)
    self.assertTrue(self.lbi.TestFile(dir_name, is_dir=True))
    self.assertTrue(self.lbi.IsDir(dir_name))

    # Delete dir.
    self.lbi.RmRF(dir_name)
    self.assertFalse(self._exists(dir_name))
    self.assertFalse(self.lbi.TestFile(dir_name, is_dir=True))
    self.assertFalse(self.lbi.IsDir(dir_name))

  @decorators.Enabled('linux', 'chromeos')
  def testRunCmdOnDevice(self):
    stdout, _ = self.lbi.RunCmdOnDevice(['echo', '$USER'])

    if not self.lbi.local:
      self.assertEqual(stdout.strip(), 'root')
    else:
      self.assertEqual(stdout.strip(), os.getenv('USER'))

  @decorators.Enabled('linux', 'chromeos')
  def testGetDeviceHostClockOffset(self):
    offset = self.lbi.GetDeviceHostClockOffset()
    second_offset = self.lbi.GetDeviceHostClockOffset()
    self.assertEqual(offset, second_offset)

    # Can only really tell if its offset to itself, with some margin of the
    # call being made.
    if self.lbi.local:
      self.assertAlmostEqual(offset, 0, places=1)

  @decorators.Enabled('linux', 'chromeos')
  def testRmRF(self):
    tmp_dir = self.lbi.MkdTemp(prefix='/tmp/foo_bar_XXXXXX')
    self.assertTrue(self._exists(tmp_dir))
    # Add a file to show its rm -rf'able
    self.lbi.RunCmdOnDevice(['touch', self.lbi.path.join(tmp_dir, 'foo.txt')])

    self.lbi.RmRF(tmp_dir)

    self.assertFalse(self._exists(tmp_dir))

  @decorators.Enabled('linux', 'chromeos')
  def testTakeScreenshots(self):
    if not self.lbi.CanTakeScreenshot():
      raise unittest.SkipTest('Cannot take screenshots. Skipping test')

    prefix = ''.join([random.choice(string.ascii_letters) for i in range(8)])
    temp_file = self._createTempFile(f'{prefix}.png')
    self.assertFalse(self.lbi.FileExistsOnDevice(temp_file))
    self.assertTrue(self.lbi.TakeScreenshot(temp_file))

    self.assertTrue(self.lbi.FileExistsOnDevice(temp_file))

    # Test with prefix.
    prefix = ''.join([random.choice(string.ascii_letters) for i in range(8)])

    # Cleanup if the file already exists.
    path_created = f'/tmp/telemetry/screenshots/{prefix}-0.png'
    if self.lbi.FileExistsOnDevice(path_created):
      self.lbi.RmRF(path_created)
    print('Creating screenshot file: %s'  %path_created)

    try:
      self.assertTrue(self.lbi.TakeScreenshotWithPrefix(prefix))
      self.assertTrue(self.lbi.FileExistsOnDevice(path_created))
    finally:
      self.lbi.RmRF(path_created)

  @decorators.Enabled('linux', 'chromeos')
  def testGetArchNameCalls(self):
    arch_name = self.lbi.GetArchName()
    self.assertTrue(arch_name)

  @decorators.Enabled('linux')
  def testLsbReleaseValue(self):
    # Should parse, and not fallback to default.
    codename = self.lbi.LsbReleaseValue('DISTRIB_CODENAME', 'foo')
    self.assertNotEqual(codename, 'foo')

    # Get Default value for unknown key.
    self.assertEqual('value not found', self.lbi.LsbReleaseValue(
        'FAKE KEY',
        'value not found'))

  @decorators.Enabled('linux', 'chromeos')
  def testExpandUserExpandsInPath(self):
    home, _ = self.lbi.RunCmdOnDevice(['echo', '$HOME'])
    home = home.strip()

    self.assertEqual(self.lbi.ExpandUser('~/foo/bar'), f'{home}/foo/bar')
