# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import unittest

from telemetry.internal.platform import gpu_device


class TestGPUDevice(unittest.TestCase):

  def testConstruction(self):
    device = gpu_device.GPUDevice(1000, 2000, 3000, 4000, 'test_vendor',
                                  'test_device', 'Intel', '100.99')
    self.assertEqual(device.vendor_id, 1000)
    self.assertEqual(device.device_id, 2000)
    self.assertEqual(device.sub_sys_id, 3000)
    self.assertEqual(device.revision, 4000)
    self.assertEqual(device.vendor_string, 'test_vendor')
    self.assertEqual(device.device_string, 'test_device')
    self.assertEqual(device.driver_vendor, 'Intel')
    self.assertEqual(device.driver_version, '100.99')

  def testFromDict(self):
    dictionary = {
        'vendor_id': 100,
        'device_id': 200,
        'sub_sys_id': 300,
        'revision': 400,
        'vendor_string': 'test_vendor_2',
        'device_string': 'test_device_2',
        'driver_vendor': 'NVIDIA',
        'driver_version': '123.45'
    }
    device = gpu_device.GPUDevice.FromDict(dictionary)
    self.assertEqual(device.vendor_id, 100)
    self.assertEqual(device.device_id, 200)
    self.assertEqual(device.sub_sys_id, 300)
    self.assertEqual(device.revision, 400)
    self.assertEqual(device.vendor_string, 'test_vendor_2')
    self.assertEqual(device.device_string, 'test_device_2')
    self.assertEqual(device.driver_vendor, 'NVIDIA')
    self.assertEqual(device.driver_version, '123.45')

  def testMissingAttrsFromDict(self):
    data = {
        'vendor_id': 1,
        'device_id': 2,
        'vendor_string': 'a',
        'device_string': 'b',
    }
    # Here we don't assert driver_vendor and driver_version because
    # --browser=reference uses an old version of Chrome that doesn't
    # include them in the gpu device dict.

    for k in data:
      data_copy = data.copy()
      del data_copy[k]
      with self.assertRaises(KeyError):
        gpu_device.GPUDevice.FromDict(data_copy)
