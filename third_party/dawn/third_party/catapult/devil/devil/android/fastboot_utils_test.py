#!/usr/bin/env python3
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Unit tests for the contents of fastboot_utils.py
"""

# pylint: disable=protected-access,unused-argument

import collections
import io
import logging
import os
import unittest
import sys

from unittest import mock

import six

sys.path.append(
    os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))

from devil.android import device_errors
from devil.android import device_utils
from devil.android import fastboot_utils
from devil.android.sdk import fastboot
from devil.utils import mock_calls

_BOARD = 'board_type'
_SERIAL = '0123456789abcdef'
_PARTITIONS = collections.OrderedDict([
    ('bootloader', fastboot_utils.PartitionInfo(
        image='bootloader*.img',
        optional=False,
        restart=True,
    )),
    ('radio', fastboot_utils.PartitionInfo(
        image='radio*.img',
        optional=False,
        restart=True,
    )),
])
_IMAGES = collections.OrderedDict([
    ('bootloader', 'bootloader.img'),
    ('radio', 'radio.img'),
])
_IMAGE_FILES = ['bootloader.img', 'radio.img']
_IMAGE_FILES_WITH_WIPE = [
    'bootloader.img', 'radio.img', 'cache.img'
]
_VALID_FILES = [_BOARD + '.zip', 'android-info.txt']
_INVALID_FILES = ['test.zip', 'android-info.txt']


def _FastbootWrapperMock(test_serial):
  fastbooter = mock.Mock(spec=fastboot.Fastboot)
  fastbooter.__str__ = mock.Mock(return_value=test_serial)
  fastbooter.Devices.return_value = [test_serial]
  return fastbooter


def _DeviceUtilsMock(test_serial):
  device = mock.Mock(spec=device_utils.DeviceUtils)
  device.__str__ = mock.Mock(return_value=test_serial)
  device.product_board = mock.Mock(return_value=_BOARD)
  device.adb = mock.Mock()
  return device


class FastbootUtilsTest(mock_calls.TestCase):
  def setUp(self):
    self.device_utils_mock = _DeviceUtilsMock(_SERIAL)
    self.fastboot_wrapper = _FastbootWrapperMock(_SERIAL)
    self.fastboot = fastboot_utils.FastbootUtils(
        device=self.device_utils_mock,
        fastbooter=self.fastboot_wrapper,
        default_timeout=2,
        default_retries=0)
    self.fastboot._board = _BOARD

  def FastbootCommandFailedError(self,
                                 args=None,
                                 output=None,
                                 status=None,
                                 msg=None):
    return mock.Mock(side_effect=device_errors.FastbootCommandFailedError(
        args, output, status, msg, str(self.device_utils_mock)))


class FastbootUtilsInitTest(FastbootUtilsTest):
  def testInitWithDeviceUtil(self):
    f = fastboot_utils.FastbootUtils(self.device_utils_mock)
    self.assertEqual(str(self.device_utils_mock), str(f._device))

  def testInitWithMissing_fails(self):
    with self.assertRaises(ValueError):
      fastboot_utils.FastbootUtils(device=None, fastbooter=None)
    with self.assertRaises(AttributeError):
      fastboot_utils.FastbootUtils('abc')


class FastbootUtilsWaitForFastbootMode(FastbootUtilsTest):

  # If this test fails by timing out after 1 second.
  @mock.patch('time.sleep', mock.Mock())
  def testWaitForFastbootMode(self):
    self.fastboot.WaitForFastbootMode()


class FastbootUtilsIsFastbootMode(FastbootUtilsTest):
  def testIsFastbootMode_True(self):
    self.assertEqual(True, self.fastboot.IsFastbootMode())

  def testIsFastbootMode_False(self):
    self.fastboot._serial = 'not' + _SERIAL
    self.assertEqual(False, self.fastboot.IsFastbootMode())


class FastbootUtilsEnableFastbootMode(FastbootUtilsTest):
  def testEnableFastbootMode(self):
    with self.assertCalls(
        (self.call.fastboot.IsFastbootMode(), False),
        self.call.fastboot._device.EnableRoot(),
        self.call.fastboot._device.adb.Reboot(to_bootloader=True),
        self.call.fastboot.WaitForFastbootMode()):
      self.fastboot.EnableFastbootMode()


class FastbootUtilsReboot(FastbootUtilsTest):
  def testReboot_bootloader(self):
    with self.assertCalls(self.call.fastboot.fastboot.RebootBootloader(),
                          self.call.fastboot.WaitForFastbootMode()):
      self.fastboot.Reboot(bootloader=True)

  def testReboot_normal(self):
    with self.assertCalls(
        self.call.fastboot.fastboot.Reboot(),
        self.call.fastboot._device.WaitUntilFullyBooted(timeout=mock.ANY)):
      self.fastboot.Reboot()


class FastbootUtilsFlashPartitions(FastbootUtilsTest):
  def testFlashPartitions(self):
    with mock.patch('os.listdir', return_value=_IMAGE_FILES):
      with self.assertCalls(
          (self.call.fastboot._VerifyBoard('test'), True),
          (self.call.fastboot.fastboot.Flash('bootloader',
                                             'test/bootloader.img')),
          (self.call.fastboot.Reboot(bootloader=True)),
          (self.call.fastboot.fastboot.Flash('radio', 'test/radio.img')),
          (self.call.fastboot.Reboot(bootloader=True))):
        self.fastboot._FlashPartitions(_PARTITIONS, 'test')

if six.PY2:
  _BUILTIN_OPEN = '__builtin__.open'
else:
  _BUILTIN_OPEN = 'builtins.open'


class FastbootUtilsVerifyBoard(FastbootUtilsTest):
  def testVerifyBoard_bothValid(self):
    mock_file = io.StringIO(u'require board=%s\n' % _BOARD)
    with mock.patch(_BUILTIN_OPEN, return_value=mock_file, create=True):
      with mock.patch('os.listdir', return_value=_VALID_FILES):
        self.assertTrue(self.fastboot._VerifyBoard('test'))

  def testVerifyBoard_BothNotValid(self):
    mock_file = io.StringIO(u'abc')
    with mock.patch(_BUILTIN_OPEN, return_value=mock_file, create=True):
      with mock.patch('os.listdir', return_value=_INVALID_FILES):
        self.assertFalse(self.assertFalse(self.fastboot._VerifyBoard('test')))

  def testVerifyBoard_FileNotFoundZipValid(self):
    with mock.patch('os.listdir', return_value=[_BOARD + '.zip']):
      self.assertTrue(self.fastboot._VerifyBoard('test'))

  def testVerifyBoard_ZipNotFoundFileValid(self):
    mock_file = io.StringIO(u'require board=%s\n' % _BOARD)
    with mock.patch(_BUILTIN_OPEN, return_value=mock_file, create=True):
      with mock.patch('os.listdir', return_value=['android-info.txt']):
        self.assertTrue(self.fastboot._VerifyBoard('test'))

  def testVerifyBoard_zipNotValidFileIs(self):
    mock_file = io.StringIO(u'require board=%s\n' % _BOARD)
    with mock.patch(_BUILTIN_OPEN, return_value=mock_file, create=True):
      with mock.patch('os.listdir', return_value=_INVALID_FILES):
        self.assertTrue(self.fastboot._VerifyBoard('test'))

  def testVerifyBoard_fileNotValidZipIs(self):
    mock_file = io.StringIO(u'require board=WrongBoard')
    with mock.patch(_BUILTIN_OPEN, return_value=mock_file, create=True):
      with mock.patch('os.listdir', return_value=_VALID_FILES):
        self.assertFalse(self.fastboot._VerifyBoard('test'))

  def testVerifyBoard_noBoardInFileValidZip(self):
    mock_file = io.StringIO(u'Regex wont match')
    with mock.patch(_BUILTIN_OPEN, return_value=mock_file, create=True):
      with mock.patch('os.listdir', return_value=_VALID_FILES):
        self.assertTrue(self.fastboot._VerifyBoard('test'))

  def testVerifyBoard_noBoardInFileInvalidZip(self):
    mock_file = io.StringIO(u'Regex wont match')
    with mock.patch(_BUILTIN_OPEN, return_value=mock_file, create=True):
      with mock.patch('os.listdir', return_value=_INVALID_FILES):
        self.assertFalse(self.fastboot._VerifyBoard('test'))


class FastbootUtilsFindAndVerifyPartitionsAndImages(FastbootUtilsTest):
  def testFindAndVerifyPartitionsAndImages_valid(self):
    partitions = collections.OrderedDict([
        ('bootloader', fastboot_utils.PartitionInfo(
            image='bootloader*.img',
            optional=False,
            restart=True,
        )),
        ('radio', fastboot_utils.PartitionInfo(
            image='radio*.img',
            optional=False,
            restart=True,
        )),
    ])
    files = [
        'bootloader-test-.img', 'radio123.img',
    ]
    img_check = collections.OrderedDict([
        ('bootloader', 'test/bootloader-test-.img'),
        ('radio', 'test/radio123.img'),
    ])
    parts_check = [
        'bootloader', 'radio',
    ]
    with mock.patch('os.listdir', return_value=files):
      imgs = fastboot_utils._FindAndVerifyPartitionsAndImages(
          partitions, 'test')
      parts = list(imgs.keys())
      self.assertDictEqual(imgs, img_check)
      self.assertListEqual(parts, parts_check)

  def testFindAndVerifyPartitionsAndImages_noFile_RequiredImage(self):
    partitions = collections.OrderedDict({
        'bootloader': fastboot_utils.PartitionInfo(
            image='bootloader*.img',
            optional=False,
            restart=True,
        ),
    })
    with mock.patch('os.listdir', return_value=['test']):
      with self.assertRaises(device_errors.FastbootCommandFailedError):
        fastboot_utils._FindAndVerifyPartitionsAndImages(partitions, 'test')

  def testFindAndVerifyPartitionsAndImages_noFile_NotRequiredImage(self):
    partitions = collections.OrderedDict({
        'foo': fastboot_utils.PartitionInfo(
            image='bootloader*.img',
            optional=True,
            restart=True,
        ),
    })
    with mock.patch('os.listdir', return_value=['test']):
      self.assertFalse(
          fastboot_utils._FindAndVerifyPartitionsAndImages(partitions, 'test'))


class FastbootUtilsFlashDevice(FastbootUtilsTest):
  def testFlashDevice_wipe(self):
    with mock.patch('os.listdir', return_value=_IMAGE_FILES_WITH_WIPE):
      with self.assertCalls(
          self.call.fastboot.EnableFastbootMode(),
          (self.call.fastboot._VerifyBoard('test'), True),
          (self.call.fastboot.fastboot.FlashAll('test', skip_reboot=True), []),
          self.call.fastboot.fastboot.Erase('userdata'),
          (self.call.fastboot._VerifyBoard('test'), True)):
        self.fastboot.FlashDevice('test', wipe=True)

  def testFlashDevice_noWipe(self):
    with mock.patch('os.listdir', return_value=_IMAGE_FILES):
      with self.assertCalls(
          self.call.fastboot.EnableFastbootMode(),
          (self.call.fastboot._VerifyBoard('test'), True),
          (self.call.fastboot.fastboot.FlashAll('test', skip_reboot=True), [])):
        self.fastboot.FlashDevice('test', wipe=False)


if __name__ == '__main__':
  logging.getLogger().setLevel(logging.DEBUG)
  unittest.main(verbosity=2)
