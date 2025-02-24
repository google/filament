#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import unittest

if __name__ == '__main__':
  sys.path.append(
      os.path.abspath(os.path.join(
          os.path.dirname(__file__),
          '..',
          '..',
      )))

from devil.android import crash_handler
from devil.android import device_errors
from devil.android import device_utils
from devil.android import device_temp_file
from devil.android import device_test_case
from devil.utils import cmd_helper
from devil.utils import reraiser_thread
from devil.utils import timeout_retry


class DeviceCrashTest(device_test_case.DeviceTestCase):
  def setUp(self):
    super(DeviceCrashTest, self).setUp()
    self.device = device_utils.DeviceUtils(self.serial, persistent_shell=False)

  def testCrashDuringCommandPersistentShellOff(self):
    self.device.EnableRoot()
    with device_temp_file.DeviceTempFile(self.device.adb) as trigger_file:

      trigger_text = 'hello world'

      def victim():
        trigger_cmd = 'echo -n %s > %s; sleep 20' % (cmd_helper.SingleQuote(
            trigger_text), cmd_helper.SingleQuote(trigger_file.name))
        crash_handler.RetryOnSystemCrash(
            lambda d: d.RunShellCommand(
                trigger_cmd,
                shell=True,
                check_return=True,
                retries=1,
                as_root=True,
                timeout=180),
            device=self.device)
        self.assertEqual(
            trigger_text,
            self.device.ReadFile(trigger_file.name, retries=0).strip())
        return True

      def crasher():
        def ready_to_crash():
          try:
            return trigger_text == self.device.ReadFile(
                trigger_file.name, retries=0).strip()
          except device_errors.CommandFailedError:
            return False

        timeout_retry.WaitFor(ready_to_crash, wait_period=2, max_tries=10)
        if not ready_to_crash():
          return False
        self.device.adb.Shell(
            'echo c > /proc/sysrq-trigger',
            expect_status=None,
            timeout=60,
            retries=0)
        return True

    self.assertEqual([True, True], reraiser_thread.RunAsync([crasher, victim]))


if __name__ == '__main__':
  device_test_case.PrepareDevices()
  unittest.main()
