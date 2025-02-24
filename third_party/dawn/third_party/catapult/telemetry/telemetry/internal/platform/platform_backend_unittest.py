# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock

from telemetry.core import platform as platform_module
from telemetry.internal.platform import platform_backend
from telemetry.internal.browser import possible_browser


class PlatformBackendTest(unittest.TestCase):
  def testGetTypExpectationsTags(self):
    pbe = platform_backend.PlatformBackend()
    pb = possible_browser.PossibleBrowser('reference_debug', 'win', False)
    with mock.patch.object(
        pbe.__class__, 'GetOSName', return_value='win'):
      with mock.patch.object(
          pbe.__class__, 'GetOSVersionName', return_value='win 10'):
        with mock.patch.object(
            pb.__class__, '_InitPlatformIfNeeded', return_value=None):
          pb._platform = platform_module.Platform(pbe)
          self.assertEqual(set(pb.GetTypExpectationsTags()),
                           {'win', 'win-10', 'reference-debug'})
