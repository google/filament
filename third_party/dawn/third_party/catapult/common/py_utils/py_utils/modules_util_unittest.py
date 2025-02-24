# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import unittest

from py_utils import modules_util


class FakeModule():
  def __init__(self, name, version):
    self.__name__ = name
    self.__version__ = version


class ModulesUitlTest(unittest.TestCase):
  def testRequireVersion_valid(self):
    numpy = FakeModule('numpy', '2.3')
    try:
      modules_util.RequireVersion(numpy, '1.0')
    except ImportError:
      self.fail('ImportError raised unexpectedly')

  def testRequireVersion_versionTooLow(self):
    numpy = FakeModule('numpy', '2.3')
    with self.assertRaises(ImportError) as error:
      modules_util.RequireVersion(numpy, '2.5')
    self.assertEqual(
        str(error.exception),
        'numpy has version 2.3, but version 2.5 or higher is required')

  def testRequireVersion_versionTooHigh(self):
    numpy = FakeModule('numpy', '2.3')
    with self.assertRaises(ImportError) as error:
      modules_util.RequireVersion(numpy, '1.0', '2.0')
    self.assertEqual(
        str(error.exception), 'numpy has version 2.3, but version'
        ' at or above 1.0 and below 2.0 is required')


if __name__ == '__main__':
  unittest.main()
