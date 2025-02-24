# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import sys
import unittest
from unittest import mock

from telemetry.internal.platform import linux_platform_backend
from telemetry.internal.platform import win_platform_backend
from telemetry.internal.platform import cros_platform_backend
from telemetry.internal.platform import mac_platform_backend

class DesktopPlatformBackendTest(unittest.TestCase):
  def testDesktopTagInTypExpectationsTags(self):
    if sys.platform.startswith('win'):
      desktop_backends = [win_platform_backend.WinPlatformBackend]
    elif sys.platform.startswith('darwin'):
      desktop_backends = [mac_platform_backend.MacPlatformBackend]
    else:
      desktop_backends = [
          linux_platform_backend.LinuxPlatformBackend,
          cros_platform_backend.CrosPlatformBackend]
    for db in desktop_backends:
      with mock.patch.object(db, 'GetOSVersionDetailString', return_value=''):
        with mock.patch.object(db, 'GetOSVersionName', return_value=''):
          backend = db()
          # Without mocking, this ends up trying to read from /etc/lsb-release,
          # which doesn't exist on Mac/Win.
          if isinstance(backend, cros_platform_backend.CrosPlatformBackend):
            backend.cri.GetBoard = mock.Mock(return_value='')
          # Need to mock the check which Windows uses to check whether the
          # device is a laptop.
          if isinstance(backend, win_platform_backend.WinPlatformBackend):
            backend.GetPcSystemType = mock.Mock(return_value='1')
          self.assertIn('desktop', backend.GetTypExpectationsTags())
