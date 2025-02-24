# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import posixpath

from telemetry.testing import browser_backend_test_case
from telemetry import decorators


class AndroidBrowserBackendTest(
    browser_backend_test_case.BrowserBackendTestCase):

  @decorators.Enabled('android')
  def testProfileDir(self):
    self.assertIsNotNone(self._browser_backend.profile_directory)

  @decorators.Enabled('android')
  def testPullMinidumps(self):
    """Test that minidumps can be pulled off the device and their mtimes set."""
    def GetDumpLocation(_=None):
      return '/sdcard/dumps/'

    platform_backend = self._browser_backend.platform_backend
    time_offset = platform_backend.GetDeviceHostClockOffset()
    platform_backend.GetDumpLocation = GetDumpLocation
    remote_path = posixpath.join(GetDumpLocation(), 'Crashpad', 'pending')
    self._browser_backend.device.RunShellCommand(['mkdir', '-p', remote_path])
    # Android's implementation of "touch" doesn't support setting time via
    # Unix timestamps, only via dates, which are affected by timezones. So,
    # figure out what the device's timestamp for January 2nd, 1970 is and use
    # that to calculate the expected local timestamp. January 2nd is used
    # instead of January 1st so that we can't get accidentally get a negative
    # timestamp if the host-device clock offset is negative.
    remote_dump_file = posixpath.join(remote_path, 'test_dump')
    self._browser_backend.device.RunShellCommand(
        ['touch', '-d', '1970-01-02T00:00:00', remote_dump_file])
    device_mtime = self._browser_backend.device.RunShellCommand(
        ['stat', '-c', '%Y', remote_dump_file], single_line=True)
    device_mtime = int(device_mtime.strip())
    try:
      self._browser_backend.PullMinidumps()
    finally:
      self._browser_backend.device.RemovePath(GetDumpLocation(), recursive=True)

    local_path = os.path.join(
        self._browser_backend._tmp_minidump_dir, 'test_dump')
    self.assertTrue(os.path.exists(local_path))
    self.assertEqual(os.path.getmtime(local_path), device_mtime - time_offset)

  @decorators.Enabled('android')
  def testPullMinidumpsOnlyNew(self):
    """Tests that a minidump is not pulled to the host if it already exists."""
    def GetDumpLocation(_=None):
      return '/sdcard/dumps/'

    local_old_dump_path = os.path.join(
        self._browser_backend._tmp_minidump_dir, 'old_dump')
    with open(local_old_dump_path, 'w'):
      pass
    old_dump_time = os.stat(local_old_dump_path).st_mtime

    platform_backend = self._browser_backend.platform_backend
    platform_backend.GetDumpLocation = GetDumpLocation
    remote_path = posixpath.join(GetDumpLocation(), 'Crashpad', 'pending')
    self._browser_backend.device.RunShellCommand(['mkdir', '-p', remote_path])
    remote_dump_file = posixpath.join(remote_path, 'new_dump')
    self._browser_backend.device.RunShellCommand(
        ['touch', '-d', '1970-01-02T00:00:00', remote_dump_file])
    remote_dump_file = posixpath.join(remote_path, 'old_dump')
    self._browser_backend.device.RunShellCommand(
        ['touch', '-d', '1970-01-02T00:00:00', remote_dump_file])

    try:
      self._browser_backend.PullMinidumps()
    finally:
      self._browser_backend.device.RemovePath(GetDumpLocation(), recursive=True)

    local_new_dump_path = os.path.join(
        self._browser_backend._tmp_minidump_dir, 'new_dump')
    self.assertTrue(os.path.exists(local_new_dump_path))
    self.assertTrue(os.path.exists(local_old_dump_path))
    # A changed mtime would mean that the dump was re-pulled
    self.assertEqual(os.stat(local_old_dump_path).st_mtime, old_dump_time)

  @decorators.Enabled('android')
  def testPullMinidumpsLockFilesIgnored(self):
    """Tests that .lock files are ignored when pulling minidumps."""
    def GetDumpLocation(_=None):
      return '/sdcard/dumps/'

    platform_backend = self._browser_backend.platform_backend
    platform_backend.GetDumpLocation = GetDumpLocation
    remote_path = posixpath.join(GetDumpLocation(), 'Crashpad', 'pending')
    self._browser_backend.device.RunShellCommand(['mkdir', '-p', remote_path])
    remote_dump_file = posixpath.join(remote_path, 'test_dump')
    remote_lock_file = posixpath.join(remote_path, 'test_file.lock')
    self._browser_backend.device.RunShellCommand(
        ['touch', remote_dump_file])
    self._browser_backend.device.RunShellCommand(
        ['touch', remote_lock_file])
    try:
      self._browser_backend.PullMinidumps()
    finally:
      self._browser_backend.device.RemovePath(GetDumpLocation(), recursive=True)

    local_path = os.path.join(
        self._browser_backend._tmp_minidump_dir, 'test_dump')
    self.assertTrue(os.path.exists(local_path))
    local_path = os.path.join(
        self._browser_backend._tmp_minidump_dir, 'test_file.lock')
    self.assertFalse(os.path.exists(local_path))

  @decorators.Enabled('android')
  def testPullMinidumpsLockedFilesIgnored(self):
    """Tests that files with associated .lock files are ignored."""
    def GetDumpLocation(_=None):
      return '/sdcard/dumps/'

    platform_backend = self._browser_backend.platform_backend
    platform_backend.GetDumpLocation = GetDumpLocation
    remote_path = posixpath.join(GetDumpLocation(), 'Crashpad', 'pending')
    self._browser_backend.device.RunShellCommand(['mkdir', '-p', remote_path])
    remote_dump_file = posixpath.join(remote_path, 'test_dump')
    remote_locked_dump_file = posixpath.join(remote_path, 'locked_dump')
    self._browser_backend.device.RunShellCommand(
        ['touch', remote_dump_file])
    self._browser_backend.device.RunShellCommand(
        ['touch', remote_locked_dump_file])
    self._browser_backend.device.RunShellCommand(
        ['touch', remote_locked_dump_file + '.lock'])
    try:
      self._browser_backend.PullMinidumps()
    finally:
      self._browser_backend.device.RemovePath(GetDumpLocation(), recursive=True)

    local_path = os.path.join(
        self._browser_backend._tmp_minidump_dir, 'test_dump')
    self.assertTrue(os.path.exists(local_path))
    local_path = os.path.join(
        self._browser_backend._tmp_minidump_dir, 'locked_dump')
    self.assertFalse(os.path.exists(local_path))
