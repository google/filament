# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock

from telemetry import decorators
from telemetry.internal.platform import win_platform_backend

class WinPlatformBackendTest(unittest.TestCase):
  @decorators.Enabled('win')
  def testTypExpectationsTagsForLaptop(self):
    backend = win_platform_backend.WinPlatformBackend()
    with mock.patch.object(backend, 'GetPcSystemType', return_value='2'):
      tags = backend.GetTypExpectationsTags()
      self.assertIn('win-laptop', tags)
