#!/usr/bin/env python
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function

import contextlib
import os
import posixpath
import random
import signal
import sys
import unittest

_CATAPULT_BASE_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))

sys.path.append(os.path.join(_CATAPULT_BASE_DIR, 'devil'))
from devil import devil_env
from devil.android import device_errors
from devil.android import device_test_case
from devil.android.sdk import adb_wrapper
from devil.utils import cmd_helper
from devil.utils import timeout_retry

_TEST_DATA_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), 'test', 'data'))
# Parameters to indicate if the adb shell should be persistent or not.
_ADB_SHELL_PARAM_LIST = [True, False]


def _hostAdbPids():
  ps_status, ps_output = cmd_helper.GetCmdStatusAndOutput(
      ['pgrep', '-l', 'adb'])

  if ps_status != 0:
    return []

  pids_and_names = (line.split() for line in ps_output.splitlines())
  pids = [int(pid) for pid, name in pids_and_names if name == 'adb']
  # Some adbs can be defunct/Zombie if the emulator they support crashes and
  # they can't be killed until their parent process dies.
  non_defunct_pids = []
  for pid in pids:
    _, ps_output = cmd_helper.GetCmdStatusAndOutput(
        ['ps', '-q', str(pid), '-o', 'state', '--no-headers'])
    if ps_output.strip() == 'Z':
      continue

    non_defunct_pids.append(pid)

  return non_defunct_pids


class AdbCompatibilityTest(device_test_case.DeviceTestCase):
  @classmethod
  def setUpClass(cls):
    custom_adb_path = os.environ.get('ADB_PATH')
    custom_deps = {
        'config_type': 'BaseConfig',
        'dependencies': {},
    }
    if custom_adb_path:
      custom_deps['dependencies']['adb'] = {
          'file_info': {
              devil_env.GetPlatform(): {
                  'local_paths': [custom_adb_path],
              },
          },
      }
    devil_env.config.Initialize(configs=[custom_deps])

  def testStartServer(self):
    # Manually kill off any instances of adb.
    adb_pids = _hostAdbPids()
    for p in adb_pids:
      os.kill(p, signal.SIGKILL)

    self.assertIsNotNone(
        timeout_retry.WaitFor(
            lambda: not _hostAdbPids(), wait_period=0.1, max_tries=10))

    # start the adb server
    start_server_status, _ = cmd_helper.GetCmdStatusAndOutput(
        [adb_wrapper.AdbWrapper.GetAdbPath(), 'start-server'])

    # verify that the server is now online
    self.assertEqual(0, start_server_status)
    self.assertIsNotNone(
        timeout_retry.WaitFor(lambda: bool(_hostAdbPids()),
                              wait_period=0.2,
                              max_tries=10))

  def testKillServer(self):
    adb_pids = _hostAdbPids()
    if not adb_pids:
      adb_wrapper.AdbWrapper.StartServer()

    adb_pids = _hostAdbPids()
    self.assertGreaterEqual(len(adb_pids), 1)

    kill_server_status, _ = cmd_helper.GetCmdStatusAndOutput(
        [adb_wrapper.AdbWrapper.GetAdbPath(), 'kill-server'])
    self.assertEqual(0, kill_server_status)

    adb_pids = _hostAdbPids()
    self.assertEqual(0, len(adb_pids))

  def testDevices(self):
    devices = adb_wrapper.AdbWrapper.Devices()
    self.assertNotEqual(0, len(devices), 'No devices found.')

  @contextlib.contextmanager
  def getTestInstance(self, persistent_shell=False):
    """Creates a real AdbWrapper instance for testing."""
    try:
      adb_instance = adb_wrapper.AdbWrapper(self.serial,
                                            persistent_shell=persistent_shell)
      yield adb_instance
    finally:
      adb_instance.KillAllPersistentAdbs()

  def testShell(self):
    for par in _ADB_SHELL_PARAM_LIST:
      with self.getTestInstance(par) as under_test:
        shell_ls_result = under_test.Shell('ls')
        self.assertIsInstance(shell_ls_result, str)
        self.assertTrue(bool(shell_ls_result))

  def testShell_failed(self):
    for par in _ADB_SHELL_PARAM_LIST:
      with self.getTestInstance(par) as under_test:
        with self.assertRaises(device_errors.AdbShellCommandFailedError):
          under_test.Shell('ls /foo/bar/baz')

  def testShell_externalStorageDefined(self):
    for par in _ADB_SHELL_PARAM_LIST:
      with self.getTestInstance(par) as under_test:
        external_storage = under_test.Shell('echo $EXTERNAL_STORAGE')
        self.assertIsInstance(external_storage, str)
        self.assertTrue(posixpath.isabs(external_storage))

  @contextlib.contextmanager
  def getTestPushDestination(self, under_test):
    """Creates a temporary directory suitable for pushing to."""
    external_storage = under_test.Shell('echo $EXTERNAL_STORAGE').strip()
    if not external_storage:
      self.skipTest('External storage not available.')
    while True:
      random_hex = hex(random.randint(0, 2**52))[2:]
      name = 'tmp_push_test%s' % random_hex
      path = posixpath.join(external_storage, name)
      try:
        under_test.Shell('ls %s' % path)
      except device_errors.AdbShellCommandFailedError:
        break
    under_test.Shell('mkdir %s' % path)
    try:
      yield path
    finally:
      under_test.Shell('rm -rf %s' % path)

  def testPush_fileToFile(self):
    for par in _ADB_SHELL_PARAM_LIST:
      with self.getTestInstance(par) as under_test:
        with self.getTestPushDestination(under_test) as push_target_directory:
          src = os.path.join(_TEST_DATA_DIR, 'push_file.txt')
          dest = posixpath.join(push_target_directory, 'push_file.txt')
          with self.assertRaises(device_errors.AdbShellCommandFailedError):
            under_test.Shell('ls %s' % dest)
          under_test.Push(src, dest)
          self.assertEqual(dest, under_test.Shell('ls %s' % dest).strip())

  def testPush_fileToDirectory(self):
    for par in _ADB_SHELL_PARAM_LIST:
      with self.getTestInstance(par) as under_test:
        with self.getTestPushDestination(under_test) as push_target_directory:
          src = os.path.join(_TEST_DATA_DIR, 'push_file.txt')
          dest = push_target_directory
          resulting_file = posixpath.join(dest, 'push_file.txt')
          with self.assertRaises(device_errors.AdbShellCommandFailedError):
            under_test.Shell('ls %s' % resulting_file)
          under_test.Push(src, dest)
          self.assertEqual(resulting_file,
                           under_test.Shell('ls %s' % resulting_file).strip())

  def testPush_directoryToDirectory(self):
    for par in _ADB_SHELL_PARAM_LIST:
      with self.getTestInstance(par) as under_test:
        with self.getTestPushDestination(under_test) as push_target_directory:
          src = os.path.join(_TEST_DATA_DIR, 'push_directory')
          dest = posixpath.join(push_target_directory, 'push_directory')
          with self.assertRaises(device_errors.AdbShellCommandFailedError):
            under_test.Shell('ls %s' % dest)
          under_test.Push(src, dest)
          self.assertEqual(
              sorted(os.listdir(src)),
              sorted(under_test.Shell('ls %s' % dest).strip().split()))

  def testPush_directoryToExistingDirectory(self):
    for par in _ADB_SHELL_PARAM_LIST:
      with self.getTestInstance(par) as under_test:
        with self.getTestPushDestination(under_test) as push_target_directory:
          src = os.path.join(_TEST_DATA_DIR, 'push_directory')
          dest = push_target_directory
          resulting_directory = posixpath.join(dest, 'push_directory')
          with self.assertRaises(device_errors.AdbShellCommandFailedError):
            under_test.Shell('ls %s' % resulting_directory)
          under_test.Shell('mkdir %s' % resulting_directory)
          under_test.Push(src, dest)
          self.assertEqual(
              sorted(os.listdir(src)),
              sorted(under_test.Shell('ls %s' % resulting_directory).split()))

  # TODO(jbudorick): Implement tests for the following:
  # taskset -c
  # devices [-l]
  # pull
  # shell
  # ls
  # logcat [-c] [-d] [-v] [-b]
  # forward [--remove] [--list]
  # jdwp
  # install [-l] [-r] [-s] [-d]
  # install-multiple [-l] [-r] [-s] [-d] [-p]
  # uninstall [-k]
  # backup -f [-apk] [-shared] [-nosystem] [-all]
  # restore
  # wait-for-device
  # get-state (BROKEN IN THE M SDK)
  # get-devpath
  # remount
  # reboot
  # reboot-bootloader
  # root
  # emu

  @classmethod
  def tearDownClass(cls):
    print('')
    print('')
    print('tested %s' % adb_wrapper.AdbWrapper.GetAdbPath())
    print('  %s' % adb_wrapper.AdbWrapper.Version())
    print('connected devices:')
    try:
      for d in adb_wrapper.AdbWrapper.Devices():
        print('  %s' % d)
    except device_errors.AdbCommandFailedError:
      print('  <failed to list devices>')
      raise
    finally:
      print('')


if __name__ == '__main__':
  sys.exit(unittest.main())
