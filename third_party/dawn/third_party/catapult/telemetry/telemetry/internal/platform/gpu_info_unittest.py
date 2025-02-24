# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import unittest

from telemetry.internal.platform import gpu_device
from telemetry.internal.platform import gpu_info


class TestGPUInfo(unittest.TestCase):

  def testConstruction(self):
    data = {
        'devices': [
            {'vendor_id': 1000, 'device_id': 2000,
             'vendor_string': 'a', 'device_string': 'b'},
            {'vendor_id': 3000, 'device_id': 4000,
             'vendor_string': 'k', 'device_string': 'l'}
        ],
        'aux_attributes': {
            'optimus': False,
            'amd_switchable': False,
            'driver_vendor': 'c',
            'driver_version': 'd',
            'driver_date': 'e',
            'gl_version_string': 'g',
            'gl_vendor': 'h',
            'gl_renderer': 'i',
            'gl_extensions': 'j',
        }
    }
    info = gpu_info.GPUInfo.FromDict(data)
    self.assertTrue(len(info.devices) == 2)
    self.assertTrue(isinstance(info.devices[0], gpu_device.GPUDevice))
    self.assertEqual(info.devices[0].vendor_id, 1000)
    self.assertEqual(info.devices[0].device_id, 2000)
    self.assertEqual(info.devices[0].vendor_string, 'a')
    self.assertEqual(info.devices[0].device_string, 'b')
    self.assertTrue(isinstance(info.devices[1], gpu_device.GPUDevice))
    self.assertEqual(info.devices[1].vendor_id, 3000)
    self.assertEqual(info.devices[1].device_id, 4000)
    self.assertEqual(info.devices[1].vendor_string, 'k')
    self.assertEqual(info.devices[1].device_string, 'l')
    self.assertEqual(info.aux_attributes['optimus'], False)
    self.assertEqual(info.aux_attributes['amd_switchable'], False)
    self.assertEqual(info.aux_attributes['driver_vendor'], 'c')
    self.assertEqual(info.aux_attributes['driver_version'], 'd')
    self.assertEqual(info.aux_attributes['driver_date'], 'e')
    self.assertEqual(info.aux_attributes['gl_version_string'], 'g')
    self.assertEqual(info.aux_attributes['gl_vendor'], 'h')
    self.assertEqual(info.aux_attributes['gl_renderer'], 'i')
    self.assertEqual(info.aux_attributes['gl_extensions'], 'j')

  def testMissingAttrsFromDict(self):
    data = {
        'devices': [{'vendor_id': 1000, 'device_id': 2000,
                     'vendor_string': 'a', 'device_string': 'b'}]
    }

    for k in data:
      data_copy = data.copy()
      del data_copy[k]
      with self.assertRaises(KeyError):
        gpu_info.GPUInfo.FromDict(data_copy)

  def testMissingDevices(self):
    data = {
        'devices': []
    }

    with self.assertRaises(Exception):
      gpu_info.GPUInfo.FromDict(data)
