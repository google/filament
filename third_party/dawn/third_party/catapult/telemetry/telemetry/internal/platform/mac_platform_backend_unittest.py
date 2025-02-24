# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock

from telemetry.core import platform as platform_module
from telemetry.internal.platform import mac_platform_backend
from telemetry.core import os_version
from telemetry import decorators


class MacPlatformBackendTest(unittest.TestCase):
  def testVersionCamparison(self):
    self.assertGreater(os_version.BIGSUR, os_version.CATALINA)
    self.assertGreater(os_version.CATALINA, os_version.MOJAVE)
    self.assertGreater(os_version.MOJAVE, os_version.HIGHSIERRA)
    self.assertGreater(os_version.HIGHSIERRA, os_version.SIERRA)
    self.assertGreater(os_version.SIERRA, os_version.ELCAPITAN)
    self.assertGreater(os_version.ELCAPITAN, os_version.YOSEMITE)
    self.assertGreater(os_version.YOSEMITE, os_version.MAVERICKS)
    self.assertGreater(os_version.MAVERICKS, os_version.SNOWLEOPARD)
    self.assertGreater(os_version.LION, os_version.LEOPARD)
    self.assertEqual(os_version.YOSEMITE, 'yosemite')
    self.assertEqual(os_version.MAVERICKS, 'mavericks')
    self.assertEqual('%s2' % os_version.MAVERICKS, 'mavericks2')
    self.assertEqual(''.join([os_version.MAVERICKS, '2']),
                     'mavericks2')
    self.assertEqual(os_version.LION.upper(), 'LION')

  @decorators.Enabled('mac')
  def testGetSystemLogSmoke(self):
    platform = platform_module.GetHostPlatform()
    self.assertTrue(platform.GetSystemLog())

  @decorators.Enabled('mac')
  def testTypExpectationsTagsIncludesMac10_11Tag(self):
    backend = mac_platform_backend.MacPlatformBackend()
    with mock.patch.object(
        backend, 'GetOSVersionName', return_value='snowleopard'):
      with mock.patch.object(
          backend, 'GetOSVersionDetailString', return_value='10.11'):
        self.assertIn('mac-10.11', backend.GetTypExpectationsTags())

  @decorators.Enabled('mac')
  def testTypExpectationsTagsIncludesMac10_12Tag(self):
    backend = mac_platform_backend.MacPlatformBackend()
    with mock.patch.object(
        backend, 'GetOSVersionName', return_value='snowleopard'):
      with mock.patch.object(
          backend, 'GetOSVersionDetailString', return_value='10.12'):
        self.assertIn('mac-10.12', backend.GetTypExpectationsTags())
