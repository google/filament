# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import unittest

from telemetry.internal.platform import gpu_device
from telemetry.internal.platform import gpu_info
from telemetry.internal.platform import system_info


class TestSystemInfo(unittest.TestCase):

  def testConstruction(self):
    data = {
        'model_name': 'MacBookPro 10.1',
        'gpu': {
            'devices': [
                {'vendor_id': 1000, 'device_id': 2000,
                 'vendor_string': 'a', 'device_string': 'b'},
            ]
        }
    }
    info = system_info.SystemInfo.FromDict(data)
    self.assertTrue(isinstance(info, system_info.SystemInfo))
    self.assertTrue(isinstance(info.gpu, gpu_info.GPUInfo))
    self.assertEqual(info.model_name, 'MacBookPro 10.1')
    self.assertTrue(len(info.gpu.devices) == 1)
    self.assertTrue(isinstance(info.gpu.devices[0], gpu_device.GPUDevice))
    self.assertEqual(info.gpu.devices[0].vendor_id, 1000)
    self.assertEqual(info.gpu.devices[0].device_id, 2000)
    self.assertEqual(info.gpu.devices[0].vendor_string, 'a')
    self.assertEqual(info.gpu.devices[0].device_string, 'b')

  def testEmptyModelName(self):
    data = {
        'model_name': '',
        'gpu': {
            'devices': [
                {'vendor_id': 1000, 'device_id': 2000,
                 'vendor_string': 'a', 'device_string': 'b'},
            ]
        }
    }
    info = system_info.SystemInfo.FromDict(data)
    self.assertEqual(info.model_name, '')

  def testMissingAttrsFromDict(self):
    data = {
        'model_name': 'MacBookPro 10.1',
        'devices': [{'vendor_id': 1000, 'device_id': 2000,
                     'vendor_string': 'a', 'device_string': 'b'}]
    }

    for k in data:
      data_copy = data.copy()
      del data_copy[k]
      with self.assertRaises(KeyError):
        system_info.SystemInfo.FromDict(data_copy)

  def testModelNameAndVersion(self):
    data = {
        'model_name': 'MacBookPro',
        'model_version': '10.1',
        'gpu': {
            'devices': [
                {'vendor_id': 1000, 'device_id': 2000,
                 'vendor_string': 'a', 'device_string': 'b'},
            ]
        }
    }
    info = system_info.SystemInfo.FromDict(data)
    self.assertEqual(info.model_name, 'MacBookPro 10.1')
