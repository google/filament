# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import unittest

from telemetry.internal.backends.chrome import gpu_compositing_checker
from telemetry.internal.platform import system_info


class GpuCompositingChecker(unittest.TestCase):
  def testAssertGpuCompositingEnabledFailed(self):
    data = {
        'model_name': 'MacBookPro 10.1',
        'gpu': {
            'devices': [
                {'vendor_id': 1000, 'device_id': 2000,
                 'vendor_string': 'a', 'device_string': 'b'},
            ],
            'feature_status': {'gpu_compositing': 'disabled'},
        }
    }
    info = system_info.SystemInfo.FromDict(data)

    with self.assertRaises(
        gpu_compositing_checker.GpuCompositingAssertionFailure):
      gpu_compositing_checker.AssertGpuCompositingEnabled(info)


  def testAssertGpuCompositingEnabledPassed(self):
    data = {
        'model_name': 'MacBookPro 10.1',
        'gpu': {
            'devices': [
                {'vendor_id': 1000, 'device_id': 2000,
                 'vendor_string': 'a', 'device_string': 'b'},
            ],
            'feature_status': {'gpu_compositing': 'enabled'},
        }
    }
    info = system_info.SystemInfo.FromDict(data)

    gpu_compositing_checker.AssertGpuCompositingEnabled(info)
