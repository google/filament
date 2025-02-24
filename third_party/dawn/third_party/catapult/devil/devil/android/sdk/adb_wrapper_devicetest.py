#!/usr/bin/env python

# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Tests for the AdbWrapper class."""
from __future__ import print_function

import os
import tempfile
import time
import unittest

from devil.android import device_test_case
from devil.android import device_errors
from devil.android.sdk import adb_wrapper


class TestAdbWrapper(device_test_case.DeviceTestCase):
  def setUp(self):
    super(TestAdbWrapper, self).setUp()
    self._adb = adb_wrapper.AdbWrapper(self.serial)
    self._adb.WaitForDevice()

  @staticmethod
  def _MakeTempFile(contents):
    """Make a temporary file with the given contents.

    Args:
      contents: string to write to the temporary file.

    Returns:
      The absolute path to the file.
    """
    fi, path = tempfile.mkstemp()
    with os.fdopen(fi, 'w') as f:
      f.write(contents)
    return path

  def testDeviceUnreachable(self):
    with self.assertRaises(device_errors.DeviceUnreachableError):
      bad_adb = adb_wrapper.AdbWrapper('device_gone')
      bad_adb.Shell('echo test')

  def testShell(self):
    output = self._adb.Shell('echo test', expect_status=0)
    self.assertEqual(output.strip(), 'test')
    output = self._adb.Shell('echo test')
    self.assertEqual(output.strip(), 'test')
    with self.assertRaises(device_errors.AdbCommandFailedError):
      self._adb.Shell('echo test', expect_status=1)

  # We need to access the device serial number here in order
  # to create the persistent shell and check the process and
  # need to access process to verify functionality.
  # pylint: disable=protected-access
  def testPersistentShell(self):
    serial = self._adb.GetDeviceSerial()
    with self._adb.PersistentShell(serial) as pshell:
      (res1, code1) = pshell.RunCommand('echo TEST')
      (res2, code2) = pshell.RunCommand('echo TEST2')
      self.assertEqual(len(res1), 1)
      self.assertEqual(res1[0], 'TEST')
      self.assertEqual(res2[-1], 'TEST2')
      self.assertEqual(code1, 0)
      self.assertEqual(code2, 0)

  def testPersistentShellWithClose(self):
    serial = self._adb.GetDeviceSerial()
    with self._adb.PersistentShell(serial) as pshell:
      (res1, code1) = pshell.RunCommand('echo TEST')
      (res2, code2) = pshell.RunCommand('echo TEST2', close=True)
      self.assertEqual(res1[0], 'TEST')
      self.assertEqual(code1, 0)
      self.assertEqual(res2[-1], 'TEST2')
      self.assertEqual(code2, 0)

    # Testing with 0 as we want to be able to differentiate between
    # a zero from the output and a zero from the exit_code.
    with self._adb.PersistentShell(serial) as pshell:
      (res3, code3) = pshell.RunCommand('echo 0', close=True)
      self.assertEqual(res3[-1], '0')
      self.assertEqual(code3, 0)

  def testPersistentShellEnsureStarted(self):
    serial = self._adb.GetDeviceSerial()
    with self._adb.PersistentShell(serial) as pshell:
      self.assertIsNotNone(pshell._process)
      pshell.Stop()
      self.assertIsNone(pshell._process)
      pshell.EnsureStarted()
      self.assertIsNotNone(pshell._process)
      # Force restart should start a new process which should have a new pid.
      pid = pshell._process.pid
      pshell.EnsureStarted(force_restart=True)
      self.assertNotEqual(pid, pshell._process.pid)

  def testPersistentShellHardStop(self):
    serial = self._adb.GetDeviceSerial()
    # When persistent shell exits, it'll cause an error when trying to flush
    # because the hardstop has killed the process.
    with self._adb.PersistentShell(serial) as pshell:
      self.assertIsNone(pshell._process.poll())
      pshell.HardStop()
      # Need time for process to actually die.
      # TODO: Convert to process.wait(timeout=1) when py2 is not needed.
      for count in range(10):
        if pshell._process.poll() is None:
          time.sleep(.1)
          if count == 9:
            raise device_errors.CommandTimeoutError(
                'Process not killed before timeout.')
        else:
          break

      self.assertIsNotNone(pshell._process.poll())
      pshell.EnsureStarted()
      res, status = pshell.RunCommand('echo TEST')
      self.assertEqual(res[0], 'TEST')
      self.assertEqual(status, 0)

  def testPersistentShellIterRunCommand(self):
    self._adb = adb_wrapper.AdbWrapper(self.serial, persistent_shell=True)
    self._adb.WaitForDevice()
    booleans = [False, True]
    for val in booleans:
      output_iter = self._adb._get_a_shell().IterRunCommand(
          'echo FOOBAR', include_status=False, close=val)
      lines = [str(x).strip() for x in output_iter]
      self.assertTrue('FOOBAR' in lines)
      # Check Status isn't included.
      self.assertFalse('0' in lines)

      output_iter = self._adb._get_a_shell().IterRunCommand('echo FOOBAR',
                                                            include_status=True,
                                                            close=val)
      lines = [str(x).strip() for x in output_iter]
      self.assertTrue('FOOBAR' in lines)
      # Check Status is included.
      self.assertEqual('0', lines[-1])

  def testPersistentShellKillAllAdbs(self):
    self._adb = adb_wrapper.AdbWrapper(self.serial, persistent_shell=True)
    self._adb.WaitForDevice()
    self.assertEqual(1, len(self._adb._all_persistent_shells))
    self.assertEqual(1, self._adb._idle_persistent_shells_queue.qsize())
    self._adb.KillAllPersistentAdbs()
    self.assertEqual(0, len(self._adb._all_persistent_shells))
    self.assertEqual(0, self._adb._idle_persistent_shells_queue.qsize())

  def testPersistentShellRunCommandClose(self):
    self._adb = adb_wrapper.AdbWrapper(self.serial, persistent_shell=True)
    self._adb.WaitForDevice()
    self.assertEqual(1, len(self._adb._all_persistent_shells))
    self.assertEqual(1, self._adb._idle_persistent_shells_queue.qsize())
    output, status = self._adb._get_a_shell().RunCommand('echo FOOBAR',
                                                         close=False)
    self.assertEqual(output[0], 'FOOBAR')
    self.assertEqual(status, 0)
    # Adb shell should have been re-added to queue when command is done.
    # All processes should be alive.
    self.assertEqual(1, self._adb._idle_persistent_shells_queue.qsize())
    found_none = False
    for shell in self._adb._all_persistent_shells:
      if shell._process is None:
        found_none = True
        break
    self.assertFalse(found_none)

    # Run a command with a close=True, and then check that a persistent shell
    # has a process set to None.
    output, status = self._adb._get_a_shell().RunCommand('echo FOOBAR',
                                                         close=True)
    self.assertEqual(output[0], 'FOOBAR')
    self.assertEqual(status, 0)
    # The adb should not have been re-added with close=True.
    self.assertEqual(0, self._adb._idle_persistent_shells_queue.qsize())
    self.assertEqual(0, len(self._adb._all_persistent_shells))

  def testPersistentShellRunCommandKeepEnds(self):
    self._adb = adb_wrapper.AdbWrapper(self.serial, persistent_shell=True)
    self._adb.WaitForDevice()
    output, status = self._adb._get_a_shell().RunCommand('echo FOOBAR')
    self.assertEqual(output[0], 'FOOBAR')
    self.assertEqual(status, 0)
    output, status = self._adb._get_a_shell().RunCommand('echo FOOBAR\n',
                                                         keepends=False)
    self.assertEqual(output[0], 'FOOBAR')
    self.assertEqual(status, 0)
    output, status = self._adb._get_a_shell().RunCommand('echo FOOBAR\n',
                                                         keepends=True)
    # Some devices have \r\n for their carriage return after commands.
    if output[0].endswith('\r\n'):
      self.assertEqual(output[0], 'FOOBAR\r\n')
    else:
      self.assertEqual(output[0], 'FOOBAR\n')

    self.assertEqual(status, 0)

  # pylint: enable=protected-access
  def testPushLsPull(self):
    path = self._MakeTempFile('foo')
    device_path = '/data/local/tmp/testfile.txt'
    local_tmpdir = os.path.dirname(path)
    self._adb.Push(path, device_path)
    files = dict(self._adb.Ls('/data/local/tmp'))
    self.assertTrue('testfile.txt' in files)
    self.assertEqual(3, files['testfile.txt'].st_size)
    self.assertEqual(self._adb.Shell('cat %s' % device_path), 'foo')
    self._adb.Pull(device_path, local_tmpdir)
    with open(os.path.join(local_tmpdir, 'testfile.txt'), 'r') as f:
      self.assertEqual(f.read(), 'foo')

  def testInstall(self):
    path = self._MakeTempFile('foo')
    with self.assertRaises(device_errors.AdbCommandFailedError):
      self._adb.Install(path)

  def testForward(self):
    with self.assertRaises(device_errors.AdbCommandFailedError):
      self._adb.Forward(0, 0)

  def testUninstall(self):
    with self.assertRaises(device_errors.AdbCommandFailedError):
      self._adb.Uninstall('some.nonexistant.package')

  def testRebootWaitForDevice(self):
    self._adb.Reboot()
    print('waiting for device to reboot...')
    while self._adb.GetState() == 'device':
      time.sleep(1)
    self._adb.WaitForDevice()
    self.assertEqual(self._adb.GetState(), 'device')
    print('waiting for package manager...')
    while True:
      try:
        android_path = self._adb.Shell('pm path android')
      except device_errors.AdbShellCommandFailedError:
        android_path = None
      if android_path and 'package:' in android_path:
        break
      time.sleep(1)

  def testRootRemount(self):
    self._adb.Root()
    for count in range(30):
      try:
        self._adb.Shell('start')
        break
      # Emulators may throw AdbCommandFailedError as they could temporarily be
      # in a device offline state when they come up.
      except (device_errors.DeviceUnreachableError,
              device_errors.AdbCommandFailedError):
        time.sleep(1)

      if count > 28:
        raise device_errors.DeviceUnreachableError(self.serial)

    self._adb.Remount()


if __name__ == '__main__':
  unittest.main()
